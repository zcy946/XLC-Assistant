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
- [x] 解析LLM的响应实现工具调用
- [x] 修复Agent设置页，llmComboBox未选中真实项的bug
- [ ] 修复配置加载前未备份导致内存被替换的bug (忘记是什么了😅(・∀・(・∀・(・∀・*))
- [x] 修复设置界面更新Agent/mcp服务器/模型后listwidget中的item未被刷新的bug
- [x] 实现设置界面添加Agent、Mcp服务器、LLM
- [x] 实现删除Agent、Mcp服务器、LLM
- [x] 实现设置-助手-对话列表右键的跳转展示功能
- [x] 修复添加新LLM|更新LLM信息时，点击设置-助手设置-模型的combobox列表没有更新的bug
- [x] 为mcpserver结构体添加`isActive`代表是否启用
- [x] 实现清除上下文
- [ ] 使用`QListView`重构简单的消息列表
- [ ] 实现新建对话
- [ ] 实现右键助手列表弹出菜单（编辑）
- [ ] 实现调用结果的展示
- [x] 加载json配置文件时，如果为空会导致触发assert

### 优化

- [ ] 将存储设置改为常规设置，创建`config.json`管理`LLMs.json`、`Agents.json`、`McpServers.json`的存储位置以及是否使用mcp服务器等参数
- [ ] 修改mcp服务器设置页，实现自定义启停mcp服务器，从而降低客户端初始化的压力
- [ ] 优化异常展示
- [ ] 在各个`getCurrentData`函数内部判空
- [x] 将`LLMService`和`McpGateway`作为单例实现
- [ ] 为`Conversation`结构体的`messages`相关操作加锁

### **进阶**

- [ ] 从数据库加载对话数据
- [ ] 使用QListView制作消息列表控件



## Note

## 1. `conversations` 表

对应 `Conversation` 结构体：

```sql
CREATE TABLE conversations (
    id TEXT PRIMARY KEY,                 -- 会话ID (UUID)
    agent_id TEXT NOT NULL,              -- 关联的 Agent ID
    summary TEXT,                        -- 会话摘要
    created_time TEXT NOT NULL,          -- 创建时间 (ISO 8601 格式: YYYY-MM-DD HH:MM:SS)
    updated_time TEXT NOT NULL,          -- 更新时间
);
```

索引建议：

```sql
CREATE INDEX idx_conversations_agent_id ON conversations(agent_id);
CREATE INDEX idx_conversations_updated_time ON conversations(updated_time);
```

------

## 2. `messages` 表

对应 `CMessage` 结构体：

```sql
CREATE TABLE messages (
    seq INTEGER PRIMARY KEY AUTOINCREMENT,   -- 消息序号，唯一递增
    id TEXT UNIQUE NOT NULL,                 -- 消息ID (UUID)
    conversation_id TEXT NOT NULL,       -- 关联的会话ID
    role TEXT NOT NULL CHECK(role IN ('USER', 'ASSISTANT', 'SYSTEM')), -- 消息角色
    text TEXT NOT NULL,                  -- 消息内容
    created_time TEXT NOT NULL,          -- 创建时间 (ISO 8601)
    avatar_file_path TEXT,               -- 头像文件路径

    FOREIGN KEY(conversation_id) REFERENCES conversations(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
```

索引建议：

```sql
CREATE INDEX idx_messages_conversation_id ON messages(conversation_id);
CREATE INDEX idx_messages_created_time ON messages(created_time);
```

------

## 3. 关系设计说明

- `conversations` 与 `messages` 是 **一对多**。
- 删除一个 `conversation` 时，相关 `messages` 自动删除（`ON DELETE CASCADE`）。
- `messages.role` 用 **枚举约束** (`CHECK`) 确保只允许三种值。
- `created_time`、`updated_time` 用 `TEXT` 存储 ISO 8601，方便 SQLite 内置函数比较。

------



## `sse_client`类中需要`try-catch`包裹的 public 函数

### 1. 初始化相关函数

**`initialize()`** - 初始化客户端连接 mcp_sse_client.cpp:53-114

- 可能抛出 `std::runtime_error`（SSE 连接失败、超时等）

### 2. 请求发送函数

**`send_request()`** - 发送请求并等待响应 mcp_sse_client.h:104-106

- 内部调用 `send_jsonrpc()`，可能抛出 `mcp_exception`

**`send_notification()`** - 发送通知消息 mcp_sse_client.h:112-114

- 同样调用 `send_jsonrpc()`，可能抛出 `mcp_exception`

### 3. 工具相关函数

**`call_tool()`** - 调用服务器工具 mcp_sse_client.h:128-130

- 内部调用 `send_request()`，可能抛出 `mcp_exception` mcp_sse_client.cpp:186-191

**`get_tools()`** - 获取可用工具列表 mcp_sse_client.h:135-137

- 内部调用 `send_request()`，可能抛出 `mcp_exception` mcp_sse_client.cpp:193-219

### 4. 资源相关函数

**`list_resources()`** - 列出可用资源 mcp_sse_client.h:149-150

- 调用 `send_request()`，可能抛出 `mcp_exception` mcp_sse_client.cpp:225-231

**`read_resource()`** - 读取资源内容 mcp_sse_client.h:155-157

- 调用 `send_request()`，可能抛出 `mcp_exception` mcp_sse_client.cpp:233-237

**`subscribe_to_resource()`** - 订阅资源变更 mcp_sse_client.h:162-164

- 调用 `send_request()`，可能抛出 `mcp_exception` mcp_sse_client.cpp:239-243

**`list_resource_templates()`** - 列出资源模板 mcp_sse_client.h:168-170

- 调用 `send_request()`，可能抛出 `mcp_exception` mcp_sse_client.cpp:245-247

### 5. 服务器能力函数

**`get_server_capabilities()`** - 获取服务器能力 mcp_sse_client.h:119-121

- 虽然只是返回缓存值，但如果在初始化失败后调用可能返回空值
