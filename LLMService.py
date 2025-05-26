from asyncio import Lock, Semaphore
from typing import Tuple, List, Optional
import qasync
import asyncio
from PySide6.QtCore import QTimer
from pydantic_ai.agent import Agent
from pydantic_ai.mcp import MCPServer, MCPServerHTTP
from pydantic_ai.messages import ModelRequest, ModelResponse
from pydantic_ai.models import Model # 仅用于类型提示，不直接实例化
from pydantic_ai.result import OutputDataT
from loguru import logger
import os

# 导入你的配置管理类和模型
from ConfigManager import ConfigManager, AppConfig, AgentConfig, MCPServer as ConfigMCPServer


class LLMService:
    # 类的私有属性定义
    __config_manager: ConfigManager
    __current_agent_config: AgentConfig # 存储当前LLMService基于的AgentConfig
    __agent: Agent[OutputDataT] # Agent 实例
    __mcp_servers_instances: List[MCPServerHTTP] # 存储MCPServerHTTP实例
    __is_mcp_servers_connected: bool # MCP 服务器运行状态
    __lock_mcp_server_state: Lock # MCP 服务器状态锁，用于同步
    __max_concurrency: int # 最大并发协程数量
    __mcp_server_task: Optional[asyncio.Task] = None # 用于存储MCP服务器的后台任务

    def __init__(self):
        """
        LLMService 的构造函数。
        它会获取 ConfigManager 单例，并根据当前的默认配置初始化 Agent 和 MCP 服务器。
        """
        self.__config_manager = ConfigManager() # 获取 ConfigManager 单例
        self.__is_mcp_servers_connected = False
        self.__lock_mcp_server_state = asyncio.Lock()
        self.__max_concurrency = 5 # 可从 AppConfig 中配置，如果需要

        # 初始化LLMService时，基于默认配置来构建 agent 和 mcp 服务器列表
        self._initialize_from_config()

        # 使用 QTimer.singleShot(0, ...) 确保在 Qt 事件循环启动后立即调度异步任务
        QTimer.singleShot(0, self.start_mcp_servers)
        logger.info("LLMService initialized with current application config.")

    def _initialize_from_config(self, agent_config_name: str = None):
        """
        根据 ConfigManager 中的配置（或指定模板名称）来初始化 LLMService 的内部组件。
        这个方法在 LLMService 启动时和配置更新后被调用。
        """
        # 1. 获取要使用的 AgentConfig
        if agent_config_name:
            new_agent_config = self.__config_manager.get_template(agent_config_name)
            if not new_agent_config:
                logger.warning(f"Agent template '{agent_config_name}' not found. Using default config.")
                new_agent_config = self.__config_manager.get_default_template()
        else:
            new_agent_config = self.__config_manager.get_default_template()
        
        self.__current_agent_config = new_agent_config
        app_config = self.__config_manager.get_current_app_config()

        # 2. 确保 API Key 在环境中设置，以便 PydanticAI Agent 能够自动拾取
        # PydanticAI 的 Agent 通常会查找特定的环境变量来获取 API Key
        # 因此，这里从 AppConfig 获取 API Key 后，将其设置到环境变量中
        # 这确保了无论 API Key 是来自 config.json 还是直接来自系统环境变量，
        # 都会被 PydanticAI 正确识别。
        if self.__current_agent_config.model.startswith("deepseek:"):
            deepseek_api_key = os.getenv("DEEPSEEK_API_KEY")
            if not deepseek_api_key:
                logger.error("DeepSeek API Key is not set in config or environment. DeepSeek model might fail.")
            os.environ["DEEPSEEK_API_KEY"] = deepseek_api_key # 设置环境变量

        elif self.__current_agent_config.model.startswith("openai:"):
            openai_api_key = os.getenv("OPENAI_API_KEY")
            if not openai_api_key:
                logger.error("OpenAI API Key is not set in config or environment. OpenAI model might fail.")
            os.environ["OPENAI_API_KEY"] = openai_api_key # 设置环境变量
        
        else:
            logger.warning(f"Model '{self.__current_agent_config.model}' has an unknown provider prefix. Please ensure its API key is set correctly in environment variables if required.")

        # 3. 获取并初始化 MCP 服务器实例列表
        self.__mcp_servers_instances = []
        for server_id in self.__current_agent_config.mounted_mcp_server_ids:
            mcp_config_server = self.__config_manager.get_mcp_server(server_id) # 从 ConfigManager 获取 MCP 服务器对象
            if mcp_config_server:
                try:
                    self.__mcp_servers_instances.append(MCPServerHTTP(mcp_config_server.url))
                    logger.info(f"Mounted MCP server instance: {mcp_config_server.name} ({mcp_config_server.url})")
                except Exception as e:
                    logger.error(f"Failed to create MCPServerHTTP instance for {mcp_config_server.id} ({mcp_config_server.url}): {e}")
            else:
                logger.warning(f"Configured MCP server ID '{server_id}' not found in global list for template '{self.__current_agent_config.model}'.")
        # 4. 创建 Agent 实例
        # PydanticAI 的 Agent 构造函数会根据 `model` 字符串（如 "provider:model_name"）
        # 和环境变量自动推断并配置底层模型。
        self.__agent = Agent(
            model=self.__current_agent_config.model,
            temperature=self.__current_agent_config.temperature,
            top_p=self.__current_agent_config.top_p,
            max_tokens=self.__current_agent_config.max_tokens,
            system_prompt=self.__current_agent_config.system_prompt,
            mcp_servers=self.__mcp_servers_instances, # 传递 MCP 服务器实例列表
            # enable_streaming=self.__current_agent_config.enable_streaming, # 如果 AgentConfig 中有此字段
            **self.__current_agent_config.model_kwargs # 传递额外的模型参数
        )
        logger.info(f"Agent re-initialized with model: {self.__current_agent_config.model} and {len(self.__mcp_servers_instances)} mounted MCP servers.")

    @qasync.asyncSlot()
    async def start_mcp_servers(self):
        """
        异步启动 MCP 服务器。
        使用 asyncio.Task 管理后台运行的服务器连接。
        """
        async with self.__lock_mcp_server_state:
            if self.__is_mcp_servers_connected:
                logger.info("MCP Servers has already been connected.")
                return

            if not self.__mcp_servers_instances:
                logger.info("No MCP servers configured to start for the current agent.")
                return

            if self.__mcp_server_task and not self.__mcp_server_task.done():
                logger.warning("MCP server task is already running.")
                return

            async def _run_servers():
                try:
                    # run_mcp_servers 是一个上下文管理器，它会保持连接直到上下文退出
                    async with self.__agent.run_mcp_servers():
                        logger.success("MCP Servers connected successfully and running.")
                        self.__is_mcp_servers_connected = True
                        await asyncio.Future() # 保持此协程运行，直到被外部取消 (例如应用关闭)
                except asyncio.CancelledError:
                    logger.info("MCP Servers connection cancelled.")
                except Exception as e:
                    logger.critical(f"Failed to connect to MCP servers: {e}")
                finally:
                    logger.info("MCP Servers disconnected.")
                    self.__is_mcp_servers_connected = False
                    self.__mcp_server_task = None # 清理任务引用

            # 在后台启动任务，不阻塞当前线程
            self.__mcp_server_task = asyncio.create_task(_run_servers())

    async def stop_mcp_servers(self):
        """
        异步停止当前正在运行的 MCP 服务器连接。
        """
        async with self.__lock_mcp_server_state:
            if self.__mcp_server_task and not self.__mcp_server_task.done():
                logger.info("Stopping MCP server task...")
                self.__mcp_server_task.cancel() # 取消后台任务
                try:
                    await self.__mcp_server_task # 等待任务完成或被取消
                except asyncio.CancelledError:
                    pass # 预期中的取消错误
                finally:
                    self.__mcp_server_task = None
                    self.__is_mcp_servers_connected = False
                    logger.success("MCP Servers stopped.")
            else:
                logger.info("MCP Servers are not running or already stopped.")

    async def execute_task(self, user_input: str, message_history: List[ModelRequest | ModelResponse] | None = None) -> Tuple[OutputDataT, List[ModelRequest | ModelResponse]]:
        """
        执行一个任务，向 AI Agent 发送用户输入并获取响应。
        使用 Semaphore 限制并发数量。
        """
        # 提示：如果 MCP 服务器配置了但未连接，这里会发出警告。
        # 根据你的业务逻辑，你可能希望在这里抛出异常，或者强制等待连接。
        if self.__mcp_servers_instances and not self.__is_mcp_servers_connected:
            logger.warning("MCP servers are configured but not connected. Attempting to run task without them.")
        
        async with asyncio.Semaphore(self.__max_concurrency):
            try:
                resp = await self.__agent.run(user_input, message_history=message_history)
                logger.success(f"Task '{user_input[:50]}...' completed. Output type: {type(resp.output)}")
                return resp.output, resp.all_messages()
            except Exception as e:
                logger.error(f"Task '{user_input[:50]}...' failed: {e}")
                # 返回错误信息和原始输入，确保类型匹配
                return user_input, [ModelRequest(role="user", content=user_input), ModelResponse(role="assistant", content=f"[Error] {str(e)}")]

    def update_agent_config(self, agent_config_name: str):
        """
        根据 ConfigManager 中指定的 Agent 模板名称，更新 LLMService 所使用的 Agent 配置。
        这会停止旧的 MCP 服务器连接，并尝试启动新的。
        """
        logger.info(f"Attempting to update LLMService to agent template: '{agent_config_name}'")
        
        # 1. 异步停止当前运行的 MCP 服务器任务，避免阻塞 UI
        asyncio.create_task(self.stop_mcp_servers()) 

        # 2. 重新初始化 Agent 和 MCP 服务器列表
        # 这会读取新的 AgentConfig，并根据其中的模型和 MCP ID 重新构建 Agent。
        self._initialize_from_config(agent_config_name)

        # 3. 异步启动新的 MCP 服务器任务
        QTimer.singleShot(0, self.start_mcp_servers)
        logger.success(f"LLMService has been updated to agent template: '{agent_config_name}'")