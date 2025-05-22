import time
from asyncio import Lock, Semaphore
from typing import Any, Tuple

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
from EventBus import EventBus


class DeepSeekApiManager(Singleton):
    __module: Model
    __mcp_servers: list[MCPServer]
    __agent: Agent[None | str]
    __is_mcp_server_running: bool
    __lock_mcp_server_state: Lock
    __max_concurrency: int
    __semaphore: Semaphore
    __task_counter: int

    def __init__(self):
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
        self.__is_mcp_server_running = False
        self.__lock_mcp_server_state = asyncio.Lock()
        self.__max_concurrency = 5
        self.__semaphore = asyncio.Semaphore(self.__max_concurrency)
        self.__task_counter = 0
        # 确保在事件循环运行后启动MCP服务器
        QTimer.singleShot(0, self.start_mcp_servers)

    @qasync.asyncSlot()
    async def start_mcp_servers(self):
        """异步启动 MCP 服务器"""
        async with self.__lock_mcp_server_state:
            if self.__is_mcp_server_running:
                logger.info("MCP Server already running")
                return
            try:
                async with self.__agent.run_mcp_servers():
                    logger.info("MCP Server started successfully")
                    self.__is_mcp_server_running = True
                    await asyncio.Future()  # 保持运行直到程序退出
            except asyncio.CancelledError:
                logger.info("MCP Server shutdown gracefully")
                self.__is_mcp_server_running = False
            except Exception as e:
                logger.critical(f"Failed to start MCP server: {e}")
                self.__is_mcp_server_running = False

    async def execute_task(self, user_input: str) -> Tuple[str, str]:
        """执行一个任务"""
        logger.debug(f"Translating text: {user_input}")
        async with self.__semaphore:
            try:
                resp = await self.__agent.run(user_input)
                logger.success(f"Task {user_input} completed: {resp.output}")
                return resp.output
            except Exception as e:
                logger.error(f"Task {user_input} failed: {e}")
                return user_input, f"[Error] {str(e)}"