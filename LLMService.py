from asyncio import Lock, Semaphore
from typing import Tuple
import qasync
import asyncio
from PySide6.QtCore import QTimer
from pydantic_ai.agent import Agent
from pydantic_ai.mcp import MCPServer, MCPServerHTTP
from pydantic_ai.models import Model
from pydantic_ai.models.openai import OpenAIModel
from pydantic_ai.providers.deepseek import DeepSeekProvider
from Singleton import Singleton
from loguru import logger


class LLMService(Singleton):
    __module: Model  # 模型
    __mcp_servers: list[MCPServer]  # mcp服务器列表
    __agent: Agent[None | str]  # agent
    __is_mcp_servers_connected: bool  # mcp服务器运行状态
    __lock_mcp_server_state: Lock  # mcp服务器状态锁
    __max_concurrency: int  # 最大协程数量

    def __init__(self):
        pass

    def init(self):
        self.__module = OpenAIModel(
            model_name="deepseek-chat",
            provider=DeepSeekProvider(api_key='sk-88a681b5432744208fce812a72396cb0'),
        )
        self.__mcp_servers = [MCPServerHTTP("http://0.0.0.0:8000/sse")]
        self.__agent = Agent(
            self.__module,
            system_prompt="你是一个助手，解决用户的需求",
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
                    logger.info("MCP Servers connected successfully")
                    self.__is_mcp_servers_connected = True
                    await asyncio.Future()  # 保持运行直到程序退出
            except asyncio.CancelledError:
                logger.info("MCP Servers disconnected")
                self.__is_mcp_servers_connected = False
            except Exception as e:
                logger.critical(f"Failed to connect to MCP servers: {e}")
                self.__is_mcp_servers_connected = False

    async def execute_task(self, user_input: str) -> Tuple[str, str]:
        """执行一个任务"""
        logger.debug(f"Translating text: {user_input}")
        async with asyncio.Semaphore(self.__max_concurrency):
            try:
                resp = await self.__agent.run(user_input)
                logger.success(f"Task {user_input} completed: {resp.output}")
                return resp.output
            except Exception as e:
                logger.error(f"Task {user_input} failed: {e}")
                return user_input, f"[Error] {str(e)}"
