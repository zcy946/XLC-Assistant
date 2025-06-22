## 📃TODO List

- [x] 分离McpServer到单独的类
- [x] 从MCPServer.json文件中加载mcp服务器
- [x] 更新MCPServer.json文件
- [x] 从Agents.json文件中加载agent
- [x] 更新Agents.json文件
- [x] 从DataManager中**异步**读取mcp服务器和agent并渲染
- [x] 实现Agents设置界面增删挂载的mcp服务器
- [x] 封装mcpclient为mcp网关，用于统一管理mcp服务器
- [x] 创建LLMService类用于不同的Agent和Mcp网关交互
- [x] 抽象出Model类用于管理模型
- [x] 将Agent的McpServers改为用QSet存储
- [ ] 实现调用LLM对话
- [ ] 修复配置加载前未备份导致内存被替换的bug
- [ ] 修复设置界面更新Agent/mcp服务器/模型后listwidget中的item未被刷新的bug
- [ ] 创建配置文件，管理`LLMs.json`、`Agents.json`、`McpServers.json`的存储位置以及是否使用mcp服务器等参数

**进阶**

- [ ] 从数据库加载对话数据
