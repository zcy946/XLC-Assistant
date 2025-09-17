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
- [x] 实现设置界面添加Agent、Mcp服务器、LLM
- [x] 实现删除Agent、Mcp服务器、LLM
- [x] 实现设置-助手-对话列表右键的跳转展示功能
- [x] 修复添加新LLM|更新LLM信息时，点击设置-助手设置-模型的combobox列表没有更新的bug
- [ ] 为mcpserver结构体添加`isActive`是否激活变量

### 优化

- [ ] 将存储设置改为常规设置，创建`config.json`管理`LLMs.json`、`Agents.json`、`McpServers.json`的存储位置以及是否使用mcp服务器等参数
- [ ] 将`McpGateway`的`callTool`等函数中的`QString`类型的`json`数据改为使用Qt的Json存储
- [ ] 修改mcp服务器设置页，实现自定义启停mcp服务器，从而降低客户端初始化的压力
- [ ] 优化异常展示
- [ ] 在各个`getCurrentData`函数内部判空
- [ ] 将`LLMService`和`McpGateway`作为单例实现

### **进阶**

- [ ] 从数据库加载对话数据
- [ ] 使用QListView制作消息列表控件



### Note

关于实现mcp服务器初始化（`initClient`函数）的相关思路：

- **当某个服务器被调用时：**检查是否存在已经被激活的mcp服务器。如果没有就将此mcp服务器的id存入`pendingClients`中，然后使用QtConcurrent配合QFutureWatcher对此mcp服务器进行初始化（具体细节为：在QtConcurrent中初始化client并将client存入McpService的client容器中，在QFutureWatcher的finished槽中将该mcp服务器的id从`pendingClients`剔除）。（pendingClients的结构`QMap<QString, QFuture<MCPClient*>> pendingClients;  `）

- **用户添加mcp服务器时：**当用户添加了新的mcp服务器后，执行mcp服务器初始化相关流程。
- **用户挂载mcp服务器时：**当用户在聊天页面挂载了mcp服务器后，检查该mcp服务器是否已经被初始化，如果没有则检查该服务器id是否存在于`pendingClients`列表中，如果存在则直接返回。如果没有被初始化则执行初始化相关流程。
- **用户获取mcp服务器工具列表或者其他信息时：**根据情况判断是否需要对相关mcp服务器进行初始化。
- **在自动检查mcp服务器是否存活相关线程中：**如果存在mcp服务器保活相关功能，则应该在例如ping了服务器后根据情况执行mcp服务器初始化流程。

**最好在QtConcurrent中调用`initClient`函数！！！**（判断返回的future是否为空，）

## QFuture 并发控制机制解释

`QMap<QString, QFuture<MCPClient*>> pendingClients` 的工作原理类似于 Cherry Studio 中的 Promise 机制： MCPService.ts:137

### 运作原理：

1. **存储异步操作**：`QFuture` 代表一个正在进行的异步初始化操作
2. **防止重复初始化**：当多个请求同时需要同一服务器时：

```c++
QFuture<MCPClient*> MCPService::initClient(const MCPServer& server) {  
    QString serverKey = getServerKey(server);  
      
    // 检查是否已有正在进行的初始化  
    if (pendingClients.contains(serverKey)) {  
        return pendingClients[serverKey]; // 返回同一个 QFuture  
    }  
      
    // 开始新的异步初始化  
    QFuture<MCPClient*> future = QtConcurrent::run([this, server]() {  
        return createMCPClient(server);  
    });  
      
    // 存储 QFuture 供其他并发请求使用  
    pendingClients[serverKey] = future;  
      
    // 完成后清理  
    QFutureWatcher<MCPClient*>* watcher = new QFutureWatcher<MCPClient*>;  
    connect(watcher, &QFutureWatcher<MCPClient*>::finished, [this, serverKey, watcher]() {  
        pendingClients.remove(serverKey);  
        watcher->deleteLater();  
    });  
    watcher->setFuture(future);  
      
    return future;  
}
```

### 并发场景示例：

- **请求 A**：调用 `initClient(server1)`，创建 QFuture 并存储
- **请求 B**：同时调用 `initClient(server1)`，发现已有 QFuture，直接返回
- **结果**：两个请求等待同一个初始化过程，避免重复连接

这种机制确保了即使多个操作同时需要同一服务器，也只会进行一次初始化，类似于 Cherry Studio 的实现。

## closeClient 函数的调用时机

### 1. 服务器移除时

当用户删除 MCP 服务器配置时，`removeServer` 方法会先检查是否存在活跃的客户端连接，如果存在则调用 `closeClient` 进行清理。

### 3. 连接检查失败时

在 `checkMcpConnectivity` 方法中，如果连接检查失败，会调用 `closeClient` 确保客户端状态清洁。

### 4. 应用程序清理时

在应用程序退出过程中，`cleanup` 方法会遍历所有客户端并调用 `closeClient` 进行资源清理。

这个 `cleanup` 方法在应用程序的 `will-quit` 事件中被调用。
