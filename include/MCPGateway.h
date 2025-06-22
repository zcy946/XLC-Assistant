#ifndef MCPGATEWAY_H
#define MCPGATEWAY_H

#include <QObject>
#include <QMap>
#include <QSet>
#include <QString>
#include <QMutex>
#include <QtConcurrent/QtConcurrent>
#include <mcp_sse_client.h>

// 这个结构体用来存储每个注册的服务器信息
struct RegisteredServer
{
    std::unique_ptr<mcp::sse_client> client;
    mcp::json available_tools; // 缓存该服务器提供的工具列表
};

class MCPGateway : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void serverRegistered(const QString &serverUuid);
    void serverUnregistered(const QString &serverUuid);
    void registrationFailed(const QString &serverUuid, const QString &error);
    void toolCallSucceeded(const QString &conversationUuid, const QString &toolName, const QString &resultJson);
    void toolCallFailed(const QString &conversationUuid, const QString &toolName, const QString &error);

public:
    explicit MCPGateway(QObject *parent = nullptr);

    mcp::json getToolsForServer(const QString &serverUuid);

    mcp::json getToolsForServers(const QSet<QString> &serverUuids);

    // 获取所有已注册服务器提供的全部工具列表
    mcp::json getAllAvailableTools();

public slots:
    // 当一个新Agent/话题启动时，调用此方法注册它的MCP服务器
    void registerServer(const QString &serverUuid, const QString &host, int port, const QString &endpoint = "/sse");
    void registerServer(const QString &serverUuid, const QString &baseUrl, const QString &endpoint = "/sse");

    // 注销服务器
    void unregisterServer(const QString &serverUuid);

    // 异步执行工具调用
    void callTool(const QString &conversationUuid, const QString &toolName, const mcp::json &params);

private:
    QMap<QString, std::shared_ptr<RegisteredServer>> m_servers;
    QMutex m_mutex;
};

#endif // MCPGATEWAY_H