#include "McpGateway.h"

McpGateway::McpGateway(QObject *parent) : QObject(parent) {}

mcp::json McpGateway::getToolsForServer(const QString &serverUuid)
{
    QMutexLocker locker(&m_mutex);
    if (m_servers.contains(serverUuid))
    {
        return m_servers[serverUuid]->available_tools;
    }
    return mcp::json::array();
}

mcp::json McpGateway::getToolsForServers(const QSet<QString> &serverUuids)
{
    QMutexLocker locker(&m_mutex);
    mcp::json merged_tools = mcp::json::array();
    for (const auto &serverUuid : serverUuids)
    {
        if (m_servers.contains(serverUuid))
        {
            for (const auto &tool : m_servers[serverUuid]->available_tools)
            {
                merged_tools.push_back(tool);
            }
        }
    }
    return merged_tools;
}

mcp::json McpGateway::getAllAvailableTools()
{
    QMutexLocker locker(&m_mutex);
    mcp::json all_tools = mcp::json::array();
    for (const auto &server : m_servers)
    {
        for (const auto &tool : server->available_tools)
        {
            all_tools.push_back(tool);
        }
    }
    return all_tools;
}

void McpGateway::registerServer(const QString &serverUuid, const QString &host, int port, const QString &endpoint)
{
    QMutexLocker locker(&m_mutex);
    if (m_servers.contains(serverUuid))
    {
        // Already registered, maybe update?
        return;
    }

    std::unique_ptr<mcp::sse_client> client = std::make_unique<mcp::sse_client>(host.toStdString(), port, endpoint.toStdString());
    if (client->initialize("GatewayClient", "0.1.0"))
    {
        auto mcpServer = std::make_shared<RegisteredServer>();
        mcpServer->client = std::move(client);
        // 获取并缓存这个服务器的工具
        for (const auto &tool : mcpServer->client->get_tools())
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
            mcpServer->available_tools.push_back(convertedTool);
        }
        m_servers.insert(serverUuid, mcpServer);
        emit serverRegistered(serverUuid); // 通知外界，例如让ChatManager刷新可用工具列表
    }
    else
    {
        // Handle error
        emit registrationFailed(serverUuid, "Failed to initialize sse client.");
    }
}

void McpGateway::registerServer(const QString &serverUuid, const QString &baseUrl, const QString &endpoint)
{
    QMutexLocker locker(&m_mutex);
    if (m_servers.contains(serverUuid))
    {
        // Already registered, maybe update?
        return;
    }
    std::unique_ptr<mcp::sse_client> client = std::make_unique<mcp::sse_client>(baseUrl.toStdString(), endpoint.toStdString());
    if (client->initialize("GatewayClient", "0.1.0"))
    {
        auto mcpServer = std::make_shared<RegisteredServer>();
        mcpServer->client = std::move(client);
        // 获取并缓存这个服务器的工具
        for (const auto &tool : mcpServer->client->get_tools())
        {
            mcp::json convertedTool = {
                {"type", "function"},
                {"function",
                 {{"name", tool.name},
                  {"description", tool.description},
                  {"parameters",
                   {{"type", "object"},
                    {"properties", tool.parameters_schema.value("properties", mcp::json::object())},
                    {"required", tool.parameters_schema.value("required", mcp::json::array())}}}}}};
            mcpServer->available_tools.push_back(convertedTool);
        }
        m_servers.insert(serverUuid, mcpServer);
        emit serverRegistered(serverUuid); // 通知外界，例如让ChatManager刷新可用工具列表
    }
    else
    {
        // Handle error
        emit registrationFailed(serverUuid, "Failed to initialize sse client.");
    }
}

// 注销服务器
void McpGateway::unregisterServer(const QString &serverUuid)
{
    QMutexLocker locker(&m_mutex);
    m_servers.remove(serverUuid);
    emit serverUnregistered(serverUuid);
}

// 异步执行工具调用
void McpGateway::callTool(const QString &conversationUuid, const QString &callId, const QString &toolName, const mcp::json &params)
{
    QtConcurrent::run(
        [this, conversationUuid, callId, toolName, params]()
        {
            QMutexLocker locker(&m_mutex);
            // 查找哪个服务器拥有这个工具
            mcp::sse_client *targetClient = nullptr;
            QString targetServerId;
            for (auto it = m_servers.begin(); it != m_servers.end(); ++it)
            {
                const auto &tools_on_server = it.value()->available_tools;
                for (const auto &tool : tools_on_server)
                {
                    if (tool["name"].get<std::string>() == toolName.toStdString())
                    {
                        targetClient = it.value()->client.get();
                        targetServerId = it.key();
                        break;
                    }
                }
                if (targetClient)
                    break;
            }

            if (targetClient)
            {
                try
                {
                    mcp::json result = targetClient->call_tool(toolName.toStdString(), params);
                    emit toolCallSucceeded(conversationUuid, callId, toolName, QString::fromStdString(result.dump()));
                }
                catch (const std::exception &e)
                {
                    emit toolCallFailed(conversationUuid, callId, toolName, QString("Execution error: %1").arg(e.what()));
                }
            }
            else
            {
                emit toolCallFailed(conversationUuid, callId, toolName, "Tool not found on any registered server.");
            }
        });
}