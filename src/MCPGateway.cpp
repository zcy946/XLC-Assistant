#include "MCPGateway.h"

MCPGateway::MCPGateway(QObject *parent) : QObject(parent) {}

mcp::json MCPGateway::getToolsForServer(const QString &serverId)
{
    QMutexLocker locker(&m_mutex);
    if (m_servers.contains(serverId))
    {
        return m_servers[serverId]->available_tools;    
    }
    return mcp::json::array(); // 返回空数组
}

mcp::json MCPGateway::getToolsForServers(const QVector<QString> &serverIds)
{
    QMutexLocker locker(&m_mutex);
    mcp::json merged_tools = mcp::json::array();
    for (const auto &serverId : serverIds)
    {
        if (m_servers.contains(serverId))
        {
            for (const auto &tool : m_servers[serverId]->available_tools)
            {
                merged_tools.push_back(tool);
            }
        }
    }
    return merged_tools;
}

mcp::json MCPGateway::getAllAvailableTools()
{
    QMutexLocker locker(&m_mutex);
    mcp::json all_tools = mcp::json::array();
    for (const auto &pair : m_servers)
    {
        for (const auto &tool : pair->available_tools)
        {
            all_tools.push_back(tool);
        }
    }
    return all_tools;
}

void MCPGateway::registerServer(const QString &serverId, const QString &host, int port)
{
    QMutexLocker locker(&m_mutex);
    if (m_servers.contains(serverId))
    {
        // Already registered, maybe update?
        return;
    }

    auto client = std::make_unique<mcp::sse_client>(host.toStdString(), port);
    if (client->initialize("GatewayClient", "0.1.0"))
    {
        auto mcpServer = std::make_shared<RegisteredServer>();
        mcpServer->client = std::move(client);
        // 获取并缓存这个服务器的工具
        for (const auto &tool : mcpServer->client->get_tools())
        {
            mcp::json convertedTool = {
                {"type", "function"},
                {"function", {{"name", tool.name}, {"description", tool.description}, {"parameters", {{"type", "object"}, {"properties", tool.parameters_schema["properties"]}, {"required", tool.parameters_schema["required"]}}}}}};
            mcpServer->available_tools.push_back(convertedTool);
        }
        m_servers.insert(serverId, mcpServer);
        emit serverRegistered(serverId); // 通知外界，例如让ChatManager刷新可用工具列表
    }
    else
    {
        // Handle error
        emit registrationFailed(serverId, "Failed to initialize client.");
    }
}

// 注销服务器
void MCPGateway::unregisterServer(const QString &serverId)
{
    QMutexLocker locker(&m_mutex);
    m_servers.remove(serverId);
    emit serverUnregistered(serverId);
}

// 异步执行工具调用
void MCPGateway::callTool(const QString &sessionId, const QString &toolName, const mcp::json &params)
{
    // 使用QtConcurrent::run在后台执行，避免阻塞网关线程
    QtConcurrent::run(
        [this, sessionId, toolName, params]()
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
                    emit toolCallSucceeded(sessionId, toolName, QString::fromStdString(result.dump()));
                }
                catch (const std::exception &e)
                {
                    emit toolCallFailed(sessionId, toolName, QString("Execution error: %1").arg(e.what()));
                }
            }
            else
            {
                emit toolCallFailed(sessionId, toolName, "Tool not found on any registered server.");
            }
        });
}