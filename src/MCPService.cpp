#include "MCPService.h"
#include <QCoreApplication>
#include "Logger.hpp"
#include <QtConcurrent>
#include <QFutureWatcher>

MCPTool::MCPTool(const QString &name, const QString &serverUuid, const mcp::json &convertedTool)
    : name(name), serverUuid(serverUuid), convertedTool(convertedTool)
{
    id = buildFunctionCallToolName(serverUuid, name);
}

QString MCPTool::buildFunctionCallToolName(const QString &serverUuid, const QString &toolName)
{
    // UUID 去掉 "-"，取前 8 字节（16 个 hex 字符）
    std::string uuidHex;
    for (char c : serverUuid.toStdString())
    {
        if (c != '-')
            uuidHex.push_back(c);
    }
    std::string prefix = uuidHex.substr(0, 16);
    // 清理 toolName
    std::string name = toolName.toStdString();
    auto trim = [](std::string &s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                        [](unsigned char ch)
                                        { return !std::isspace(ch); }));
        s.erase(std::find_if(s.rbegin(), s.rend(),
                             [](unsigned char ch)
                             { return !std::isspace(ch); })
                    .base(),
                s.end());
    };
    trim(name);
    std::replace(name.begin(), name.end(), '-', '_');
    // 拼接 UUID 前缀
    name = prefix + "-" + name;
    // 替换非法字符
    for (auto &ch : name)
    {
        if (!(std::isalnum(static_cast<unsigned char>(ch)) || ch == '_' || ch == '-'))
        {
            ch = '_';
        }
    }
    // 确保以字母开头
    if (name.empty() || !std::isalpha(static_cast<unsigned char>(name[0])))
    {
        name = "tool-" + name;
    }
    // 去掉连续分隔符
    name = std::regex_replace(name, std::regex("(_|-){2,}"), "_");
    // 限制长度
    if (name.size() > 63)
    {
        name = name.substr(0, 63);
    }
    // 去掉末尾的 "_" 或 "-"
    while (!name.empty() && (name.back() == '_' || name.back() == '-'))
    {
        name.pop_back();
    }
    return QString::fromStdString(name);
}

MCPService *MCPService::s_instance = nullptr;

MCPService *MCPService::getInstance()
{
    if (!s_instance)
    {
        s_instance = new MCPService();
        // 在应用程序退出时自动清理单例实例
        connect(qApp, &QCoreApplication::aboutToQuit, s_instance, &QObject::deleteLater);
    }
    return s_instance;
}

MCPService::MCPService(QObject *parent)
    : QObject(parent)
{
}

std::shared_ptr<MCPClient> MCPService::createStdioClient(std::shared_ptr<McpServer> server)
{
    // 拼接完整命令
    QString fullCommand(server->command);
    for (const QString &arg : server->args)
    {
        fullCommand.append(" ").append(arg);
    }
    // 拼接环境变量
    mcp::json environmentVariables;
    for (auto it = server->envVars.constBegin(); it != server->envVars.constEnd(); ++it)
    {
        environmentVariables[it.key().toStdString()] = it.value().toStdString();
    }
    // 创建stdio client
    std::unique_ptr<mcp::stdio_client> client = std::make_unique<mcp::stdio_client>(fullCommand.toStdString(), environmentVariables);
    // 设置capabilities
    mcp::json capabilities = {{"roots", {{"listChanged", true}}}};
    client->set_capabilities(capabilities);
    try
    {
        // 初始化连接
        XLC_LOG_DEBUG("正在初始化与MCP服务器 [{}] 的连接...", server->uuid);
        if (!client->initialize("XLCClient", mcp::MCP_VERSION))
        {
            XLC_LOG_ERROR("未能初始化与MCP服务器 [{}] 的连接", server->uuid);
            return nullptr;
        }
        // Ping 服务器
        XLC_LOG_DEBUG("正在 ping MCP服务器 [{}]...", server->uuid);
        if (!client->ping())
        {
            std::cerr << "Failed to ping server" << std::endl;
            XLC_LOG_ERROR("未能 ping 到MCP服务器 [{}]", server->uuid);
            return nullptr;
        }
        // 获取capabilities
        XLC_LOG_DEBUG("正在获取MCP服务器 [{}] 的capabilities...", server->uuid);
        mcp::json capabilities = client->get_server_capabilities();
        XLC_LOG_DEBUG("MCP服务器 [{}] 的capabilities: {}", server->uuid, capabilities.dump(4));
        // 创建 MCPClient 对象
        auto mcpClient = std::make_shared<MCPClient>();
        // 获取tools
        XLC_LOG_DEBUG("正在获取MCP服务器 [{}] 的tools...", server->uuid);
        mcpClient->tools = registerTools(server->uuid, client.get());
        XLC_LOG_DEBUG("获取到 {} 个来自MCP服务器 [{}] 的tool", mcpClient->tools.size(), server->uuid);
        mcpClient->client = std::move(client);
        return mcpClient;
    }
    catch (const mcp::mcp_exception &e)
    {
        XLC_LOG_ERROR("MCPServer [{}] error: {} - {}", static_cast<int>(e.code()), e.what());
        return nullptr;
    }
    catch (const std::exception &e)
    {
        XLC_LOG_ERROR("MCPServer [{}] error: {}", e.what());
        return nullptr;
    }
}

std::shared_ptr<MCPClient> MCPService::createSSEClient(std::shared_ptr<McpServer> server)
{
    std::unique_ptr<mcp::sse_client> client;
    if (!server->baseUrl.isEmpty())
    {
        if (server->endpoint.isEmpty())
            client = std::make_unique<mcp::sse_client>(server->baseUrl.toStdString(), "/sse");
        else
            client = std::make_unique<mcp::sse_client>(server->baseUrl.toStdString(), server->endpoint.toInt());
    }
    else if (!server->host.isEmpty() && server->port != 0)
    {
        if (server->endpoint.isEmpty())
            client = std::make_unique<mcp::sse_client>(server->host.toStdString(), server->port);
        else
            client = std::make_unique<mcp::sse_client>(server->host.toStdString(), server->port, server->endpoint.toStdString());
    }
    // 设置capabilities
    mcp::json capabilities = {{"roots", {{"listChanged", true}}}};
    client->set_capabilities(capabilities);
    // 设置timeout
    client->set_timeout(server->timeout);
    try
    {
        // 初始化连接
        XLC_LOG_DEBUG("正在初始化与MCP服务器 [{}] 的连接...", server->uuid);
        if (!client->initialize("XLCClient", mcp::MCP_VERSION))
        {
            XLC_LOG_ERROR("未能初始化与MCP服务器 [{}] 的连接", server->uuid);
            return nullptr;
        }
        // Ping 服务器
        XLC_LOG_DEBUG("正在 ping MCP服务器 [{}]...", server->uuid);
        if (!client->ping())
        {
            std::cerr << "Failed to ping server" << std::endl;
            XLC_LOG_ERROR("未能 ping 到MCP服务器 [{}]", server->uuid);
            return nullptr;
        }
        // 获取capabilities
        XLC_LOG_DEBUG("正在获取MCP服务器 [{}] 的capabilities...", server->uuid);
        mcp::json capabilities = client->get_server_capabilities();
        XLC_LOG_DEBUG("MCP服务器 [{}] 的capabilities: {}", server->uuid, capabilities.dump(4));
        // 创建 MCPClient 对象
        auto mcpClient = std::make_shared<MCPClient>();
        // 获取tools
        XLC_LOG_DEBUG("正在获取MCP服务器 [{}] 的tools...", server->uuid);
        mcpClient->tools = registerTools(server->uuid, client.get());
        XLC_LOG_DEBUG("获取到 {} 个来自MCP服务器 [{}] 的tool", mcpClient->tools.size(), server->uuid);
        mcpClient->client = std::move(client);
        return mcpClient;
    }
    catch (const mcp::mcp_exception &e)
    {
        XLC_LOG_ERROR("MCPServer [{}] error: {} - {}", static_cast<int>(e.code()), e.what());
        return nullptr;
    }
    catch (const std::exception &e)
    {
        XLC_LOG_ERROR("MCPServer [{}] error: {}", e.what());
        return nullptr;
    }
}

std::shared_ptr<MCPClient> MCPService::createMCPClient(const QString &serverUuid)
{
    std::shared_ptr<McpServer> mcpServer = DataManager::getInstance()->getMcpServer(serverUuid);
    switch (mcpServer->type)
    {
    case McpServer::Type::stdio:
    {
        return createStdioClient(mcpServer);
        break;
    }
    case McpServer::Type::sse:
    {
        return createSSEClient(mcpServer);
        break;
    }
    case McpServer::Type::streambleHttp:
    {
        // 暂不支持的服务器类型
        XLC_LOG_WARN("暂不支持的MCPServer类型: {} - streambleHttp", static_cast<int>(mcpServer->type));
        break;
    }
    default:
    {
        XLC_LOG_WARN("未知MCPServer类型: {}", static_cast<int>(mcpServer->type));
        break;
    }
    }
    return nullptr;
}

QVector<QString> MCPService::registerTools(const QString &serverUuid, mcp::client *client)
{
    QVector<QString> tools;
    QMutexLocker locker(&m_mutexTools);
    for (const auto &tool : client->get_tools())
    {
        // 如果键不存在，则分别使用空的 JSON 对象和空的 JSON 数组作为默认值
        mcp::json convertedTool = {
            {"type", "function"},
            {"function",
             {{"name", tool.name},
              {"description", tool.description},
              {"parameters",
               {{"type", "object"},
                {"properties", tool.parameters_schema.value("properties", mcp::json::object())},
                {"required", tool.parameters_schema.value("required", mcp::json::array())}}}}}};
        std::shared_ptr<MCPTool> mcpTool = std::make_shared<MCPTool>(QString::fromStdString(tool.name), serverUuid, convertedTool);
        tools.push_back(mcpTool->id);
        // 更新工具列表
        m_tools.insert(mcpTool->id, mcpTool);
    }
    return tools;
}

void MCPService::initClient(const QString &serverUuid)
{
    XLC_LOG_DEBUG("尝试为服务器 [{}] 初始化客户端。", serverUuid);

    // 检查客户端是否已经初始化完成
    {
        QMutexLocker locker(&m_mutexClients);
        if (m_clients.contains(serverUuid))
        {
            XLC_LOG_DEBUG("服务器 [{}] 的客户端已就绪，无需重复初始化。", serverUuid);
            // 如果需要，可以在这里重新发射 sig_clientReady 信号，通知新的监听者客户端已就绪。
            // Q_EMIT sig_clientReady(serverUuid, m_clients[serverUuid]);
            return;
        }
    }

    // 检查客户端是否正在初始化中
    {
        QMutexLocker locker(&m_mutexPendingClients);
        if (m_pendingClients.contains(serverUuid))
        {
            XLC_LOG_DEBUG("服务器 [{}] 的客户端正在初始化中，等待现有过程完成。", serverUuid);
            // 如果有多个调用者，并且都想知道何时完成，他们可以连接到现有的 future 的 watcher，
            // 但为了简化，这里只是跳过重复启动。
            return;
        }
    }

    XLC_LOG_INFO("为服务器 [{}] 启动新的异步客户端初始化。", serverUuid);

    // 启动新的异步初始化任务
    QFuture<std::shared_ptr<MCPClient>> future = QtConcurrent::run(
        [this, serverUuid]()
        {
            return createMCPClient(serverUuid);
        });

    {
        QMutexLocker locker(&m_mutexPendingClients);
        m_pendingClients.insert(serverUuid, future);
    }

    QFutureWatcher<std::shared_ptr<MCPClient>> *watcher = new QFutureWatcher<std::shared_ptr<MCPClient>>();
    connect(watcher, &QFutureWatcher<std::shared_ptr<MCPClient>>::finished, this,
            [this, serverUuid, watcher]()
            {
                {
                    QMutexLocker locker(&m_mutexPendingClients);
                    m_pendingClients.remove(serverUuid);
                }

                QFuture<std::shared_ptr<MCPClient>> finishedFuture = watcher->future();
                if (finishedFuture.isFinished())
                {
                    std::shared_ptr<MCPClient> client = finishedFuture.result();
                    if (client)
                    {
                        XLC_LOG_DEBUG("服务器 [{}] 的客户端初始化成功！", serverUuid);
                        {
                            QMutexLocker locker(&m_mutexClients);
                            m_clients.insert(serverUuid, client); // 存储已就绪的客户端
                        }
                        Q_EMIT sig_clientReady(serverUuid, client);
                    }
                    else
                    {
                        XLC_LOG_WARN("服务器 [{}] 的客户端初始化失败（createMCPClient 返回 nullptr）！", serverUuid);
                        Q_EMIT sig_clientError(serverUuid, "初始化失败：客户端对象为空或创建失败。");
                    }
                }
                else if (finishedFuture.isCanceled())
                {
                    XLC_LOG_WARN("服务器 [{}] 的客户端初始化被取消。", serverUuid);
                    Q_EMIT sig_clientError(serverUuid, "初始化被取消。");
                }
                else
                {
                    // 对于 QtConcurrent::run 来说，通常不会出现这种情况，但为了健壮性考虑
                    XLC_LOG_WARN("服务器 [{}] 的 Future 已完成但结果不可用或被取消（异常状态）。", serverUuid);
                    Q_EMIT sig_clientError(serverUuid, "Future 状态异常。");
                }
                watcher->deleteLater(); // 销毁 watcher
            });
    watcher->setFuture(future);
}

void MCPService::closeClient(const QString &serverUuid)
{
    // 从m_clients中清除client
    std::shared_ptr<MCPClient> client;
    {
        QMutexLocker locker(&m_mutexClients);
        auto it_Client = m_clients.find(serverUuid);
        if (it_Client == m_clients.end())
            return;
        client = it_Client.value();
        m_clients.erase(it_Client);
    }

    // 从m_tools中清除工具
    {
        QMutexLocker locker(&m_mutexTools);
        for (const QString &toolId : client->tools)
        {
            m_tools.remove(toolId);
        }
    }
}

void MCPService::callTool(const CallToolArgs &callToolArgs)
{
    // 获取MCPTool
    std::shared_ptr<MCPTool> mcpTool;
    {
        QMutexLocker locker(&m_mutexTools);
        auto it_McpTool = m_tools.find(callToolArgs.toolName);
        if (it_McpTool == m_tools.end())
        {
            XLC_LOG_WARN("{} 调用失败，不存在的tool: {}", callToolArgs.callId, callToolArgs.toolName);
            return;
        }
        mcpTool = it_McpTool.value();
    }

    // 获取MCPClient
    std::shared_ptr<MCPClient> mcpClient;
    {
        QMutexLocker locker(&m_mutexClients);
        auto it_McpClient = m_clients.find(mcpTool->serverUuid);
        if (it_McpClient == m_clients.end())
        {
            XLC_LOG_WARN("{} 调用失败，不存在的mcp客户端: {}", callToolArgs.callId, mcpTool->serverUuid);
            return;
        }
        mcpClient = it_McpClient.value();
    }

    if (!mcpClient->tools.contains(callToolArgs.toolName))
    {
        XLC_LOG_WARN("{} 调用失败，mcp客户端 {} 中不存在名为 {} 的tool", callToolArgs.callId, mcpTool->serverUuid, callToolArgs.toolName);
        return;
    }

    // 异步调用tool
    QtConcurrent::run(
        [mcpClient, mcpTool, callToolArgs]()
        {
            try
            {
                mcp::json result = mcpClient->client->call_tool(mcpTool->name.toStdString(), callToolArgs.parameters);
                // TODO 调用成功
                XLC_LOG_TRACE("{} - {} 调用成功: {}", callToolArgs.callId, mcpTool->name, result.dump(4));
            }
            // TODO 调用失败
            catch (const mcp::mcp_exception &e)
            {
                XLC_LOG_ERROR("{} - {} 调用失败，MCP error: {}", callToolArgs.callId, mcpTool->name, e.what());
            }
            catch (const std::exception &e)
            {
                XLC_LOG_ERROR("{} - {} 调用失败，Standard error: {}", callToolArgs.callId, mcpTool->name, e.what());
            }
            catch (...)
            {
                XLC_LOG_ERROR("{} - {} 调用失败，Unknown error occurred", callToolArgs.callId, mcpTool->name);
            }
        });
}

mcp::json MCPService::getToolsFromServer(const QString &serverUuid)
{
    std::shared_ptr<MCPClient> client;
    {
        QMutexLocker locker(&m_mutexClients);
        auto it_McpClient = m_clients.find(serverUuid);
        if (it_McpClient == m_clients.end())
            return mcp::json::array();
        client = it_McpClient.value();
    }

    QMutexLocker locker(&m_mutexTools);
    mcp::json toolsJson = mcp::json::array();
    for (const QString &toolId : client->tools)
    {
        auto it_McpTool = m_tools.find(toolId);
        if (it_McpTool != m_tools.end())
        {
            toolsJson.push_back((*it_McpTool)->convertedTool);
        }
    }
    return toolsJson;
}