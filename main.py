# import asyncio
# import os
# import sys
# from loguru import logger
# from pydantic_ai import Agent
# from pydantic_ai.models.openai import OpenAIModel
# from pydantic_ai.providers.deepseek import DeepSeekProvider
# from pydantic_ai.mcp import MCPServerStdio
# from pydantic_ai.mcp import MCPServerHTTP
# from aioconsole import ainput
#
# model = OpenAIModel(
#     model_name="deepseek-chat",
#     provider=DeepSeekProvider(api_key='sk-88a681b5432744208fce812a72396cb0'),
# )
#
# # server = MCPServerStdio(
# #     command='D:/Anaconda3/python.exe',
# #     args=["E:/学习资料/作业/人工智能实训/PY/ShallowSeek/mcp/mcp_main.py"],
# # )
# server = MCPServerHTTP("http://0.0.0.0:8000/sse")
#
# agent = Agent(model,
#               system_prompt="你是一位经验丰富的程序员",
#               mcp_servers=[server])
#
#
# async def main():
#     history = []
#     try:
#         # 启动 MCP 服务器
#         async with agent.run_mcp_servers():
#             while True:
#                 # 异步接收用户输入，防止阻塞
#                 user_input = await ainput("(输入`exit`退出)>: ")
#                 if user_input.lower() == 'exit':
#                     break
#                 try:
#                     # 同步运行 Agent
#                     resp = await agent.run(user_input, message_history=history)
#                     history = list(resp.all_messages())
#                     logger.success("AI: {}", resp.output)
#                 except KeyboardInterrupt:
#                     logger.info("正在退出...")
#                 except Exception as e:
#                     logger.error(f"处理输入时出错：{e}", file=sys.stderr)
#     except Exception as e:
#         logger.critical(f"启动MCP服务器或Agent失败: {e}")
#
#
# def initLogger():
#     """初始化日志"""
#     output_dir = "output"
#     os.makedirs(output_dir, exist_ok=True)
#     logger.add(os.path.join(output_dir, "ShallowSeek_Client.log"), enqueue=True, rotation="10 MB", retention=5)
#
#
# if __name__ == "__main__":
#     initLogger()
#     try:
#         asyncio.run(main())
#     except KeyboardInterrupt:
#         logger.info("程序已由用户终止")
#     except RuntimeError as e:
#         logger.critical(f"事件循环错误：{e}")
#         sys.exit(1)
#     except Exception as e:
#         logger.critical(f"未知错误：{e}")
#         sys.exit(1)


import asyncio
import os
import sys
from loguru import logger
from pydantic_ai import Agent
from pydantic_ai.models.openai import OpenAIModel
from pydantic_ai.providers.deepseek import DeepSeekProvider
from pydantic_ai.mcp import MCPServerHTTP
import textwrap
from typing import List, Tuple

# 初始化模型和服务器（保持不变）
model = OpenAIModel(
    model_name="deepseek-chat",
    provider=DeepSeekProvider(api_key='sk-88a681b5432744208fce812a72396cb0'),
)

server = MCPServerHTTP("http://0.0.0.0:8000/sse")

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


async def translate_segment(segment: str, index: int, semaphore: asyncio.Semaphore) -> Tuple[int, str]:
    """翻译单个文本段"""
    async with semaphore:
        try:
            # 不传递 message_history，关闭上下文
            resp = await agent.run(f"Translate the following English text to Chinese:\n{segment}")
            logger.success(f"Segment {index} translated: {resp.output}")
            return index, resp.output
        except Exception as e:
            logger.error(f"Error translating segment {index}: {e}")
            return index, f"[Error] {str(e)}"


async def translate_text(text: str, max_concurrency: int = 5, segment_size: int = 500) -> str:
    """将文本分段并并行翻译"""
    # 分割文本（按固定字符长度）
    segments = textwrap.wrap(text, width=segment_size, break_long_words=False)
    logger.info(f"Text split into {len(segments)} segments")

    # 创建信号量，限制并发数量
    semaphore = asyncio.Semaphore(max_concurrency)

    # 创建并行翻译任务
    tasks = [
        translate_segment(segment, i, semaphore) for i, segment in enumerate(segments)
    ]

    # 等待所有任务完成
    results = await asyncio.gather(*tasks, return_exceptions=True)

    # 按索引排序结果，确保翻译顺序正确
    sorted_results = sorted(results, key=lambda x: x[0])
    translated_text = "\n".join(result[1] for result in sorted_results)

    return translated_text


async def main():
    try:
        # 启动 MCP 服务器
        async with agent.run_mcp_servers():
            # 示例文本（可以替换为你的输入）
            sample_text = """
            Buying a house involves several key steps, from financial readiness to closing the deal. First, assess your financial situation, save for a down payment and closing costs, and get pre-approved for a mortgage. Then, find a real estate agent and start looking for properties, making an offer when you find the right one. A home inspection and appraisal follow, and you'll negotiate with the seller before securing financing and finalizing the sale. 
            Here's a more detailed breakdown:
            1. Financial Readiness:
            Determine your budget: Calculate how much house you can afford based on your income and debt. 
            Save for a down payment and closing costs: Aim for at least 20% down payment to avoid private mortgage insurance (PMI). 
            Get pre-approved for a mortgage: This shows you're serious and helps you understand your loan options and how much you can borrow. 
            2. Finding a Real Estate Agent and a Home:
            Choose a real estate agent:
            They can help you find properties that fit your needs and negotiate the deal. 
            Start your house search:
            Look at properties online and in person, considering your budget, location, and needs. 
            Make an offer:
            Once you find a home you love, your agent will help you make an offer, which may be accepted or counter-offered. 
            3. Inspection, Appraisal, and Negotiation:
            Get a home inspection: This helps identify any potential problems or repairs needed.
            Get an appraisal: This determines the fair market value of the property.
            Negotiate with the seller: If necessary, discuss repairs or credits based on the inspection and appraisal. 
            4. Securing Financing and Closing the Deal:
            Secure your mortgage: Your lender will provide the funds to purchase the home.
            Do a final walk-through: Ensure the property is in the agreed-upon condition before closing.
            Close on the house: Sign the final paperwork and officially become the homeowner. 
            """  # 模拟长文本

            # 并行翻译
            logger.info("Starting translation...")
            translated = await translate_text(
                text=sample_text,
                max_concurrency=5,  # 控制最大并发数
                segment_size=500  # 每段最大字符数
            )
            logger.success("Translation completed:")
            print("Translated text:\n", translated)

    except Exception as e:
        logger.critical(f"Failed to start MCP server or run Agent: {e}")


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
