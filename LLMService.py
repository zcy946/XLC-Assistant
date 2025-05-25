from asyncio import Lock, Semaphore
from typing import Tuple
import qasync
import asyncio
from PySide6.QtCore import QTimer
from pydantic_ai.agent import Agent
from pydantic_ai.mcp import MCPServer, MCPServerHTTP
from pydantic_ai.messages import ModelRequest, ModelResponse
from pydantic_ai.models import Model
from pydantic_ai.models.openai import OpenAIModel
from pydantic_ai.providers.deepseek import DeepSeekProvider
from loguru import logger
from pydantic_ai.result import OutputDataT
import os
from dotenv import load_dotenv


class LLMService:
    __module: Model  # 模型
    __system_prompt: str  # 系统提示词
    __mcp_servers: list[MCPServer]  # mcp服务器列表
    __agent: Agent[None | str]  # agent
    __is_mcp_servers_connected: bool  # mcp服务器运行状态
    __lock_mcp_server_state: Lock  # mcp服务器状态锁
    __max_concurrency: int  # 最大协程数量

    def __init__(self):
        # 加载 .env 文件中的环境变量
        load_dotenv()
        deepseek_api_key = os.environ.get("DEEPSEEK_API_KEY")
        if not deepseek_api_key:
            raise ValueError("DEEPSEEK_API_KEY environment variable not set in .env or environment.")
        self.__module = OpenAIModel(
            model_name="deepseek-chat",
            provider=DeepSeekProvider(api_key=deepseek_api_key),
        )
        self.__system_prompt = "你是一个助手，解决用户的需求"
        self.__mcp_servers = [MCPServerHTTP("http://0.0.0.0:8000/sse")]
        self.__agent = Agent(
            self.__module,
            system_prompt=self.__system_prompt,
            mcp_servers=self.__mcp_servers
        )
        self.__is_mcp_servers_connected = False
        self.__lock_mcp_server_state = asyncio.Lock()
        self.__max_concurrency = 5
        # 确保在事件循环运行后启动MCP服务器
        QTimer.singleShot(0, self.start_mcp_servers)

    @qasync.asyncSlot()
    async def start_mcp_servers(self):
        """异步启动 MCP 服务器"""
        async with self.__lock_mcp_server_state:
            if self.__is_mcp_servers_connected:
                logger.info("MCP Servers has already been connected")
                return
            try:
                async with self.__agent.run_mcp_servers():
                    logger.success("MCP Servers connected successfully")
                    self.__is_mcp_servers_connected = True
                    await asyncio.Future()  # 保持运行直到程序退出
            except asyncio.CancelledError:
                logger.info("MCP Servers disconnected")
                self.__is_mcp_servers_connected = False
            except Exception as e:
                logger.critical(f"Failed to connect to MCP servers: {e}")
                self.__is_mcp_servers_connected = False

    async def execute_task(self, user_input: str, message_history: list | None) -> tuple[OutputDataT, list[ModelRequest | ModelResponse]] | tuple[str, str]:
        """执行一个任务"""
        async with asyncio.Semaphore(self.__max_concurrency):
            try:
                resp = await self.__agent.run(user_input, message_history=message_history)
                logger.success(f"Task {user_input[:50]}... completed: {resp.output}")
                return resp.output, resp.all_messages()
            except Exception as e:
                logger.error(f"Task {user_input[:50]}... failed: {e}")
                return user_input, f"[Error] {str(e)}"

    def set_api_key(self, api_key):
        """设置api key"""
        self.__module = OpenAIModel(
            model_name="deepseek-chat",
            provider=DeepSeekProvider(api_key=api_key),
        )
        self.__agent = Agent(
            self.__module,
            system_prompt=self.__system_prompt,
            mcp_servers=self.__mcp_servers
        )
        logger.info("Api key has been updated")

    def set_system_prompt(self, new_system_prompt: object) -> None:
        """设置提示词"""
        if not new_system_prompt:
            return
        self.__system_prompt = new_system_prompt
        self.__agent = Agent(
            self.__module,
            system_prompt=self.__system_prompt,
            mcp_servers=self.__mcp_servers
        )
        logger.info(f"System prompt has been updated to: {new_system_prompt[:50]}...")

    def set_mcp_server_list(self, addresses: list[str]) -> None:
        """设置mcp服务器列表"""
        if len(addresses) == 0:
            return
        for address in addresses:
            self.__mcp_servers.append(MCPServerHTTP(address))
        self.__agent = Agent(
            self.__module,
            system_prompt=self.__system_prompt,
            mcp_servers=self.__mcp_servers
        )
        logger.info(f"MCP Servers has been updated")
