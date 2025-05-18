from mcp.server.fastmcp import FastMCP
import tools

mcp = FastMCP("host info mcp")
mcp.add_tool(tools.get_host_info)
mcp.add_tool(tools.read_file)
mcp.add_tool(tools.list_files)
mcp.add_tool(tools.rename_file)

# 同文件内可以通过注释直接添加
# @mcp.tool()
# def foo():
#     return ""

def main():
    mcp.run("stdio") # sse


if __name__ == "__main__":
    main()