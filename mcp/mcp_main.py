from mcp.server.fastmcp import FastMCP
import tools
import os
from loguru import logger

mcp = FastMCP("ShallowSeek MCP")


# 同文件内可以通过注释直接添加
# @mcp.tool()
# def foo():
#     return ""

def initTools():
    """初始化工具函数"""
    mcp.add_tool(tools.read_file)
    mcp.add_tool(tools.list_files)
    mcp.add_tool(tools.rename_file)


def initLogger():
    """初始化日志"""
    output_dir = "output"
    os.makedirs(output_dir, exist_ok=True)
    logger.add(os.path.join(output_dir, "ShallowSeek_MCPServer.log"), enqueue=True, rotation="10 MB", retention=5)


def init():
    """初始化"""
    initLogger()
    initTools()


def main():
    init()
    logger.info("Start MCP server...")
    mcp.run("stdio")


if __name__ == "__main__":
    main()
