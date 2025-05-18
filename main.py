import asyncio
import sys

from pydantic_ai import Agent
from pydantic_ai.models.openai import OpenAIModel
from pydantic_ai.providers.deepseek import DeepSeekProvider
from pydantic_ai.mcp import MCPServerStdio

import tools

model = OpenAIModel(
    model_name="deepseek-chat",
    provider=DeepSeekProvider(api_key='sk-88a681b5432744208fce812a72396cb0'),
)

server = MCPServerStdio(
    command='D:/Anaconda3/python.exe',
    args=["E:/学习资料/作业/人工智能实训/PY/ShallowSeek/mcp/mcp_main.py"],
)

agent = Agent(model,
              system_prompt="你是一位经验丰富的程序员",
              # tools=[tools.read_file, tools.list_files, tools.rename_file],
              mcp_servers=[server])


async def main():
    history = []
    try:
        # 启动 MCP 服务器
        async with agent.run_mcp_servers():
            while True:
                user_input = input("Input (type 'exit' to quit): ")
                if user_input.lower() == 'exit':
                    break
                try:
                    # 同步运行 Agent
                    resp = await agent.run(user_input, message_history=history)
                    history = list(resp.all_messages())
                    print("Output:", resp.output)
                except Exception as e:
                    print(f"处理输入时出错：{e}", file=sys.stderr)
    except Exception as e:
        print(f"Error starting MCP server or agent: {e}")


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except RuntimeError as e:
        print(f"事件循环错误：{e}", file=sys.stderr)
        sys.exit(1)
