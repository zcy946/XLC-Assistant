import asyncio
import os
import sys
from loguru import logger
from pydantic_ai import Agent
from pydantic_ai.models.openai import OpenAIModel
from pydantic_ai.providers.deepseek import DeepSeekProvider
from pydantic_ai.mcp import MCPServerHTTP
from aioconsole import ainput
from typing import Tuple
import time


model = OpenAIModel(
    model_name="deepseek-chat",
    provider=DeepSeekProvider(api_key='sk-88a681b5432744208fce812a72396cb0'),
)

server = MCPServerHTTP("http://0.0.0.0:8000/sse")

# 初始化Agent
agent = Agent(
    model,
    system_prompt="你是一位专业的翻译专家，请将输入的英文文本翻译成中文。",
    mcp_servers=[server]
)

def initLogger():
    """初始化日志"""
    output_dir = "output"
    os.makedirs(output_dir, exist_ok=True)
    logger.add(os.path.join(output_dir, "ShallowSeek_Client.log"), enqueue=True, rotation="10 MB", retention=5)

async def translate_task(text: str, task_id: str, semaphore: asyncio.Semaphore) -> Tuple[str, str]:
    """执行单个翻译任务"""
    async with semaphore:
        try:
            # 不传递 message_history，关闭上下文
            resp = await agent.run(f"Translate the following English text to Chinese:\n{text}")
            # logger.success(f"Task {task_id} completed: {resp.output}")
            return task_id, resp.output
        except Exception as e:
            logger.error(f"Task {task_id} failed: {e}")
            return task_id, f"[Error] {str(e)}"


async def main():
    # 最大并发数
    MAX_CONCURRENCY = 5
    semaphore = asyncio.Semaphore(MAX_CONCURRENCY)

    # 任务计数器，用于生成唯一任务 ID
    task_counter = 0

    try:
        # 启动 MCP 服务器
        async with agent.run_mcp_servers():
            logger.info("Started listening for user input. Enter text to translate (or 'exit' to quit).")
            print("Enter English text to translate (or 'exit' to quit):")
            while True:
                """
                NOTE
                    async with 条件变量:
                        await 条件变量.wait_for(lambda:  任务数 > 0 || 停止)
                            if (停止)
                                break
                            执行任务
                            
                """
                # 异步等待用户输入
                user_input = await ainput("> ")

                # 检查是否退出
                if user_input.lower() == 'exit':
                    logger.info("User requested exit")
                    break

                # 忽略空输入
                if not user_input.strip():
                    logger.warning("Empty input ignored")
                    continue

                # 生成任务 ID（使用计数器和时间戳）
                task_counter += 1
                task_id = f"{task_counter}_{int(time.time())}"
                logger.info(f"Received new task {task_id}: {user_input[:50]}...")

                # 创建翻译任务
                asyncio.create_task(
                    handle_task_result(translate_task(user_input, task_id, semaphore))
                )

    except Exception as e:
        logger.critical(f"Failed to start MCP server or run Agent: {e}")


async def handle_task_result(task: asyncio.Task) -> None:
    """处理翻译任务的结果"""
    task_id, result = await task
    logger.success(f"\n[Task {task_id}] Translated text:\n{result}\n")
    logger.info("Enter next text to translate (or 'exit' to quit):")
    # NOTE 可以根据taskid(topicid)来对上下文进行存储


if __name__ == "__main__":
    initLogger()
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logger.info("Program terminated by user")
    except RuntimeError as e:
        logger.critical(f"Event loop error: {e}")
        sys.exit(1)
    except Exception as e:
        logger.critical(f"Unknown error: {e}")
        sys.exit(1)