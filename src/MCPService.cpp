#include "MCPService.h"
#include <QCoreApplication>
#include "Logger.hpp"
#include <QtConcurrent>
#include <QFutureWatcher>

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
        // 获取tools
        XLC_LOG_DEBUG("正在获取MCP服务器 [{}] 的tools...", server->uuid);
        mcp::json availableTools;
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
            availableTools.push_back(convertedTool);
        }
        XLC_LOG_DEBUG("获取到 {} 个来自MCP服务器 [{}] 的tool", availableTools.size(), server->uuid);

        // 创建并返回 MCPClient 对象
        auto mcpClient = std::make_shared<MCPClient>();
        mcpClient->client = std::move(client);
        mcpClient->availableTools = availableTools;
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
        // 获取tools
        XLC_LOG_DEBUG("正在获取MCP服务器 [{}] 的tools...", server->uuid);
        mcp::json availableTools;
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
            availableTools.push_back(convertedTool);
        }
        XLC_LOG_DEBUG("获取到 {} 个来自MCP服务器 [{}] 的tool", availableTools.size(), server->uuid);

        // 创建并返回 MCPClient 对象
        auto mcpClient = std::make_shared<MCPClient>();
        mcpClient->client = std::move(client);
        mcpClient->availableTools = availableTools;
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

void MCPService::initClient(const QString &serverUuid)
{
    XLC_LOG_DEBUG("尝试为服务器 [{}] 初始化客户端。", serverUuid);

    // 检查客户端是否已经初始化完成
    if (m_clients.contains(serverUuid))
    {
        XLC_LOG_DEBUG("服务器 [{}] 的客户端已就绪，无需重复初始化。", serverUuid);
        // 如果需要，可以在这里重新发射 sig_clientReady 信号，通知新的监听者客户端已就绪。
        // Q_EMIT sig_clientReady(serverUuid, m_clients[serverUuid]);
        return;
    }

    // 检查客户端是否正在初始化中
    if (m_pendingClients.contains(serverUuid))
    {
        XLC_LOG_DEBUG("服务器 [{}] 的客户端正在初始化中，等待现有过程完成。", serverUuid);
        // 如果有多个调用者，并且都想知道何时完成，他们可以连接到现有的 future 的 watcher，
        // 但为了简化，这里只是跳过重复启动。
        return;
    }

    XLC_LOG_INFO("为服务器 [{}] 启动新的异步客户端初始化。", serverUuid);

    // 启动新的异步初始化任务
    QFuture<std::shared_ptr<MCPClient>> future = QtConcurrent::run(
        [this, serverUuid]()
        {
            // 在工作线程中执行实际的客户端创建和连接逻辑
            return createMCPClient(serverUuid);
        });

    // 将 future 存储在 pending 列表中，表示正在进行中
    m_pendingClients.insert(serverUuid, future);

    // 创建一个 QFutureWatcher 来监听 future 的完成状态
    QFutureWatcher<std::shared_ptr<MCPClient>> *watcher = new QFutureWatcher<std::shared_ptr<MCPClient>>();
    connect(watcher, &QFutureWatcher<std::shared_ptr<MCPClient>>::finished, this,
            [this, serverUuid, watcher]()
            {
                // 初始化完成后，从 pending 列表中移除
                m_pendingClients.remove(serverUuid);

                QFuture<std::shared_ptr<MCPClient>> finishedFuture = watcher->future();
                if (finishedFuture.isFinished())
                {
                    std::shared_ptr<MCPClient> client = finishedFuture.result();
                    if (client)
                    {
                        XLC_LOG_DEBUG("服务器 [{}] 的客户端初始化成功！", serverUuid);
                        m_clients.insert(serverUuid, client); // 存储已就绪的客户端
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
