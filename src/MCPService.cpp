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
    // 注册自定义类型（在不同线程之间安全地传递自定义数据，需要通过 qRegisterMetaType() 显式地告诉 Qt 如何处理这些数据类型。）
    qRegisterMetaType<CallToolArgs>("CallToolArgs");
    qRegisterMetaType<mcp::json>("mcp::json");
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
        XLC_LOG_DEBUG("Initializing connection (MCPServer={})", server->uuid);
        if (!client->initialize("XLCClient", mcp::MCP_VERSION))
        {
            XLC_LOG_ERROR("Failed to initialize connection to MCP server (MCPServer={})", server->uuid);
            return nullptr;
        }
        // Ping 服务器
        XLC_LOG_DEBUG("Pinging MCP server (MCPServer={})", server->uuid);
        if (!client->ping())
        {
            XLC_LOG_ERROR("Failed ping to MCP server (MCPServer={})", server->uuid);
            return nullptr;
        }
        // 获取capabilities
        XLC_LOG_DEBUG("Getting capabilities for MCP server (MCPServer={})", server->uuid);
        mcp::json capabilities = client->get_server_capabilities();
        XLC_LOG_DEBUG("MCP server capabilities (MCPServer={}): {}", server->uuid, capabilities.dump(4));
        // 创建 MCPClient 对象
        auto mcpClient = std::make_shared<MCPClient>();
        // 获取tools
        XLC_LOG_DEBUG("Getting tools for MCP server (MCPServer={})", server->uuid);
        mcpClient->tools = registerTools(server->uuid, client.get());
        XLC_LOG_DEBUG("Retrieved tools from MCP server (count={}, serverUuid={})", mcpClient->tools.size(), server->uuid);
        mcpClient->client = std::move(client);
        return mcpClient;
    }
    catch (const mcp::mcp_exception &e)
    {
        XLC_LOG_ERROR("Create stdio client failed (code={}, message={}): mcp error", static_cast<int>(e.code()), e.what());
        return nullptr;
    }
    catch (const std::exception &e)
    {
        XLC_LOG_ERROR("Create stdio client failed (message={})", e.what());
        return nullptr;
    }
}

std::shared_ptr<MCPClient> MCPService::createSSEClient(std::shared_ptr<McpServer> server)
{
    std::unique_ptr<mcp::sse_client> client;
    if (!server->baseUrl.isEmpty())
    {
        // base URL
        if (server->endpoint.isEmpty())
            client = std::make_unique<mcp::sse_client>(server->baseUrl.toStdString(), "/sse");
        else
            client = std::make_unique<mcp::sse_client>(server->baseUrl.toStdString(), server->endpoint.toStdString());
    }
    else if (!server->host.isEmpty() && server->port != 0)
    {
        // host + port
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
        // NOTE 当服务器为localhost时，客户端填写127.0.0.1将无法连接到服务器必须添加localhost；反过来则没有这个问题
        // 初始化连接
        XLC_LOG_DEBUG("Initializing connection to MCP server (MCPServer={})", server->uuid);
        if (!client->initialize("XLCClient", mcp::MCP_VERSION))
        {
            XLC_LOG_ERROR("Failed initialize connection to MCP server (MCPServer={}", server->uuid);
            return nullptr;
        }
        // Ping 服务器
        XLC_LOG_DEBUG("Pinging MCP server (MCPServer={})", server->uuid);
        if (!client->ping())
        {
            XLC_LOG_ERROR("Failed ping to MCP server (MCPServer={})", server->uuid);
            return nullptr;
        }
        // 获取capabilities
        XLC_LOG_DEBUG("Getting capabilities for MCP server (MCPServer={})", server->uuid);
        mcp::json capabilities = client->get_server_capabilities();
        XLC_LOG_DEBUG("MCP server capabilities ({}): {}", server->uuid, capabilities.dump(4));
        // 创建 MCPClient 对象
        auto mcpClient = std::make_shared<MCPClient>();
        // 获取tools
        XLC_LOG_DEBUG("Attempting get tools from MCP server (MCPServer={})", server->uuid);
        mcpClient->tools = registerTools(server->uuid, client.get());
        XLC_LOG_DEBUG("Retrieved tools from MCP server (count={}, serverUuid={})", mcpClient->tools.size(), server->uuid);
        mcpClient->client = std::move(client);
        return mcpClient;
    }
    catch (const mcp::mcp_exception &e)
    {
        XLC_LOG_ERROR("Create sse client failed (code={}, message={}): mcp error", static_cast<int>(e.code()), e.what());
        return nullptr;
    }
    catch (const std::exception &e)
    {
        XLC_LOG_ERROR("Create sse client failed (message={})", e.what());
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
        XLC_LOG_WARN("Unsupported MCPServer type: {} - streambleHttp", static_cast<int>(mcpServer->type));
        break;
    }
    default:
    {
        XLC_LOG_WARN("Unknown MCPServer type: {}", static_cast<int>(mcpServer->type));
        break;
    }
    }
    return nullptr;
}

QVector<QString> MCPService::registerTools(const QString &serverUuid, mcp::client *client)
{
    try
    {
        QVector<QString> tools;
        QMutexLocker locker(&m_mutexTools);
        for (const auto &tool : client->get_tools())
        {
            std::shared_ptr<MCPTool> mcpTool = std::make_shared<MCPTool>(QString::fromStdString(tool.name), serverUuid);
            mcp::json convertedTool = {
                {"type", "function"},
                {"function",
                 {{"name", mcpTool->id.toStdString()},
                  {"description", tool.description},
                  {"parameters",
                   {{"type", "object"},
                    {"properties", tool.parameters_schema.value("properties", mcp::json::object())},
                    {"required", tool.parameters_schema.value("required", mcp::json::array())}}}}}};
            mcpTool->convertedTool = convertedTool;
            tools.push_back(mcpTool->id);
            // 更新工具列表
            m_tools.insert(mcpTool->id, mcpTool);
        }
        return tools;
    }
    catch (const mcp::mcp_exception &e)
    {
        XLC_LOG_ERROR("Register tools failed (mcp error={})", e.what());
        return QVector<QString>();
    }
    catch (const std::exception &e)
    {
        XLC_LOG_ERROR("Register tools failed (error={})", e.what());
        return QVector<QString>();
    }
}

void MCPService::initClient(const QString &serverUuid)
{
    if (!DataManager::getInstance()->getMcpServer(serverUuid))
    {
        XLC_LOG_DEBUG("Initialize client failed (serverUuid={}): MCP server not found", serverUuid);
        return;
    }
    // 检查客户端是否已经初始化完成
    {
        QMutexLocker locker(&m_mutexClients);
        if (m_clients.contains(serverUuid))
        {
            XLC_LOG_DEBUG("Client is ready, no need for re-initialization (server={})", serverUuid);
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
            XLC_LOG_DEBUG("Client is initializing (MCPServer={})", serverUuid);
            // 如果有多个调用者，并且都想知道何时完成，他们可以连接到现有的 future 的 watcher，
            // 但为了简化，这里只是跳过重复启动。
            return;
        }
    }

    XLC_LOG_DEBUG("Initializing MCP server (serverUuid={})", serverUuid);

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
                        XLC_LOG_INFO("Client initialization succeeded (serverUuid={})", serverUuid);
                        {
                            QMutexLocker locker(&m_mutexClients);
                            m_clients.insert(serverUuid, client); // 存储已就绪的客户端
                        }
                        Q_EMIT sig_clientReady(serverUuid, client);
                    }
                    else
                    {
                        XLC_LOG_WARN("Client initialization failed (serverUuid={})", serverUuid);
                        Q_EMIT sig_clientError(serverUuid, "初始化失败：客户端对象为空或创建失败。");
                    }
                }
                else if (finishedFuture.isCanceled())
                {
                    XLC_LOG_WARN("Client initialization cancelled (serverUuid={})", serverUuid);
                    Q_EMIT sig_clientError(serverUuid, "初始化被取消。");
                }
                else
                {
                    // 对于 QtConcurrent::run 来说，通常不会出现这种情况，但为了健壮性考虑
                    XLC_LOG_WARN("Server future completed but result unavailable or cancelled (exception state) (serverUuid={})", serverUuid);
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
            QString errorMessage = QString("Call tool failed (callId=%1, tool=%2): tool not found").arg(callToolArgs.callId).arg(callToolArgs.toolName);
            XLC_LOG_WARN("{}", errorMessage);
            Q_EMIT sig_toolCallFinished(callToolArgs, false, mcp::json(), errorMessage);
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
            QString errorMessage = QString("Call tool failed (callId=%1, server=%2): mcp client not found").arg(callToolArgs.callId).arg(mcpTool->serverUuid);
            XLC_LOG_WARN("{}", errorMessage);
            Q_EMIT sig_toolCallFinished(callToolArgs, false, mcp::json(), errorMessage);
            return;
        }
        mcpClient = it_McpClient.value();
    }

    if (!mcpClient->tools.contains(callToolArgs.toolName))
    {
        QString errorMessage = QString("Call tool failed (callId=%1, server=%2, tool=%3): original tool not found").arg(callToolArgs.callId).arg(mcpTool->serverUuid).arg(callToolArgs.toolName);
        XLC_LOG_WARN("{}", errorMessage);
        Q_EMIT sig_toolCallFinished(callToolArgs, false, mcp::json(), errorMessage);
        return;
    }

    // 异步调用tool
    QtConcurrent::run(
        [this, mcpClient, mcpTool, callToolArgs]()
        {
            try
            {
                mcp::json result = mcpClient->client->call_tool(mcpTool->name.toStdString(), callToolArgs.parameters);
                // 根据 isError 字段判断是否调用成功
                if (result.contains("isError") && result["isError"].is_boolean() && !result["isError"])
                {
                    // 调用成功
                    XLC_LOG_TRACE("Call tool succeeded (callId={}, tool={}): {}", callToolArgs.callId, mcpTool->name, result.dump(4));
                    // 处理调用结果
                    Q_EMIT sig_toolCallFinished(callToolArgs, true, result, Q_NULLPTR);
                }
                else
                {
                    // 调用失败
                    QString errorMessage = QString("Call tool failed (callId=%1, tool=%2): %3")
                                               .arg(callToolArgs.callId)
                                               .arg(mcpTool->name)
                                               .arg("no error details available");
                    // 如果有 content 字段，并且内容是字符串，提取错误信息
                    if (result.contains("content") && result["content"].is_array() && !result["content"].empty() &&
                        result["content"][0].contains("text") && result["content"][0]["text"].is_string())
                    {
                        errorMessage = QString("Call tool failed (callId=%1, tool=%2): %3")
                                           .arg(callToolArgs.callId)
                                           .arg(mcpTool->name)
                                           .arg(QString::fromStdString(result["content"][0]["text"].get<std::string>()));
                    }
                    XLC_LOG_ERROR("{}", errorMessage);
                    Q_EMIT sig_toolCallFinished(callToolArgs, false, mcp::json(), errorMessage);
                }
            }
            // 调用失败
            catch (const mcp::mcp_exception &e)
            {
                QString errorMessage = QString("Call tool failed (callId=%1, tool=%2): mcp error=%3")
                                           .arg(callToolArgs.callId)
                                           .arg(mcpTool->name)
                                           .arg(e.what());
                XLC_LOG_WARN("{}", errorMessage);
                Q_EMIT sig_toolCallFinished(callToolArgs, false, mcp::json(), errorMessage);
            }
            catch (const std::exception &e)
            {
                QString errorMessage = QString("Call tool failed (callId=%1, tool=%2): standard error=%3")
                                           .arg(callToolArgs.callId)
                                           .arg(mcpTool->name)
                                           .arg(e.what());

                XLC_LOG_WARN("{}", errorMessage);
                Q_EMIT sig_toolCallFinished(callToolArgs, false, mcp::json(), errorMessage);
            }
            catch (...)
            {
                QString errorMessage = QString("Call tool failed (callId=%1, tool=%2): no error details available")
                                           .arg(callToolArgs.callId)
                                           .arg(mcpTool->name);

                XLC_LOG_WARN("{}", errorMessage);
                Q_EMIT sig_toolCallFinished(callToolArgs, false, mcp::json(), errorMessage);
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
        {
            QString errorMsg;
            {
                QMutexLocker lockerPendingClients(&m_mutexPendingClients);
                if (!m_pendingClients.contains(serverUuid))
                {
                    initClient(serverUuid);
                    errorMsg = QString("Get tools failed (serverUuid={}): client not initialized, calling initClient()");
                }
            }
            errorMsg = QString("Get tools failed (serverUuid={}): client is initializing");
            XLC_LOG_WARN("errorMsg", serverUuid);
            return mcp::json::array();
        }
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

mcp::json MCPService::getToolsFromServers(const QSet<QString> mcpServers)
{
    mcp::json toolsJson = mcp::json::array();
    for (const QString mcpServerUuid : mcpServers)
    {
        mcp::json serverTools = getToolsFromServer(mcpServerUuid);
        toolsJson.insert(toolsJson.end(), serverTools.begin(), serverTools.end());
    }
    XLC_LOG_TRACE("Get tools succeeded (toolJsonStr={})", toolsJson.dump(4));
    return toolsJson;
}

// void MCPService::checkMcpConnectivity(const QString &serverUuid)
// {
//     QFuture<bool> future = QtConcurrent::run(
//         [this, serverUuid]()
//         {
//             QMutexLocker lockerClients(&m_mutexClients);
//             auto it = m_clients.find(serverUuid);
//             if (it != m_clients.end())
//             {
//                 try
//                 {
//                     it.value()->client->get_tools();
//                     XLC_LOG_INFO("Check MCP connectivity successed (serverUuid={})", serverUuid);
//                     return true;
//                 }
//                 catch (const mcp::mcp_exception &e)
//                 {
//                     XLC_LOG_INFO("Check MCP connectivity failed (serverUuid={}, errormsg={}): mcp error", serverUuid, e.what());
//                     return false;
//                 }
//                 catch (const std::exception &e)
//                 {
//                     XLC_LOG_INFO("Check MCP connectivity failed (serverUuid={}, errormsg={}): error", serverUuid, e.what());
//                     return false;
//                 }
//             }
//             else
//             {
//                 XLC_LOG_INFO("Check MCP connectivity failed (serverUuid={}): client not found", serverUuid);
//                 return false;
//             }
//         });

//     QFutureWatcher<bool> *watcher = new QFutureWatcher<bool>();
//     connect(watcher, &QFutureWatcher<bool>::finished, this,
//             [this, watcher, serverUuid]()
//             {
//                 Q_EMIT sig_checkMcpConnectivityFinished(serverUuid, watcher->future().result());
//                 watcher->deleteLater();
//             });
//     watcher->setFuture(future);
// }

bool MCPService::isInitialized(const QString &serverUuid)
{
    QMutexLocker locker(&m_mutexClients);
    if (m_clients.contains(serverUuid))
        return true;
    return false;
}