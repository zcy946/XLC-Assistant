import asyncio
import os
import sys
from loguru import logger
from pydantic_ai import Agent
from pydantic_ai.models.openai import OpenAIModel
from pydantic_ai.providers.deepseek import DeepSeekProvider
from pydantic_ai.mcp import MCPServerStdio
from pydantic_ai.mcp import MCPServerHTTP
from aioconsole import ainput

model = OpenAIModel(
    model_name="deepseek-chat",
    provider=DeepSeekProvider(api_key='sk-88a681b5432744208fce812a72396cb0'),
)

# server = MCPServerStdio(
#     command='D:/Anaconda3/python.exe',
#     args=["E:/学习资料/作业/人工智能实训/PY/ShallowSeek/mcp/mcp_main.py"],
# )
server = MCPServerHTTP("http://0.0.0.0:8000/sse")

agent = Agent(model,
              system_prompt="你是一位经验丰富的程序员",
              mcp_servers=[server])


async def main():
    history = []
    try:
        # 启动 MCP 服务器
        async with agent.run_mcp_servers():
            while True:
                # 异步接收用户输入，防止阻塞
                user_input = await ainput("(输入`exit`退出)>: ")
                if user_input.lower() == 'exit':
                    break
                try:
                    # 同步运行 Agent
                    resp = await agent.run(user_input, message_history=history)
                    history = list(resp.all_messages())
                    logger.success("AI: {}", resp.output)
                except KeyboardInterrupt:
                    logger.info("正在退出...")
                except Exception as e:
                    logger.error(f"处理输入时出错：{e}", file=sys.stderr)
    except Exception as e:
        logger.critical(f"启动MCP服务器或Agent失败: {e}")


def initLogger():
    """初始化日志"""
    output_dir = "output"
    os.makedirs(output_dir, exist_ok=True)
    logger.add(os.path.join(output_dir, "ShallowSeek_Client.log"), enqueue=True, rotation="10 MB", retention=5)


if __name__ == "__main__":
    initLogger()
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logger.info("程序已由用户终止")
    except RuntimeError as e:
        logger.critical(f"事件循环错误：{e}")
        sys.exit(1)
    except Exception as e:
        logger.critical(f"未知错误：{e}")
        sys.exit(1)
