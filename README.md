## 📃TODO List

### 常规

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
- [x] 实现调用LLM对话
- [x] 修复无`tools`时，程序报错的bug
- [ ] 解析LLM的响应实现工具调用
- [ ] 加入`max_retries`来防止llm陷入调用死循环
- [ ] 实现调用结果的展示
- [x] 修复Agent设置页，llmComboBox未选中真实项的bug
- [ ] 修复配置加载前未备份导致内存被替换的bug
- [x] 修复设置界面更新Agent/mcp服务器/模型后listwidget中的item未被刷新的bug
- [ ] 实现设置界面添加Agent、Mcp服务器、LLM
- [x] 实现设置-助手-对话列表右键的跳转展示功能

### 优化

- [ ] 将存储设置改为常规设置，创建`config.json`管理`LLMs.json`、`Agents.json`、`McpServers.json`的存储位置以及是否使用mcp服务器等参数
- [ ] 将`McpGateway`的`callTool`等函数中的`QString`类型的`json`数据改为使用Qt的Json存储
- [ ] 修改mcp服务器设置页，实现自定义启停mcp服务器，从而降低客户端初始化的压力
- [ ] 优化异常展示
- [ ] 将`LLMService`和`McpGateway`作为单例实现

### **进阶**

- [ ] 从数据库加载对话数据
- [ ] 使用QListView制作消息列表控件
