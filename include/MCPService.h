#ifndef MCPSERVICE_H
#define MCPSERVICE_H

#include <QObject>
#include <QMap>
#include <memory>
#include <mcp_sse_client.h>
#include <QFuture>

struct MCPClient
{
    std::unique_ptr<mcp::sse_client> client;
    mcp::json available_tools; // 缓存该服务器提供的工具列表
};

class MCPService : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void clientReady(const QString &mcpServerUuid, MCPClient *client);
    void clientError(const QString &mcpServerUuid, const QString &errorMessage);

public:
    /**
     * this.listTools = this.listTools.bind(this)
        this.callTool = this.callTool.bind(this)
        this.listPrompts = this.listPrompts.bind(this)
        this.getPrompt = this.getPrompt.bind(this)
        this.listResources = this.listResources.bind(this)
        this.getResource = this.getResource.bind(this)
        this.closeClient = this.closeClient.bind(this)
     */
    static MCPService *getInstance();
    ~MCPService() = default;
    void initClient(const QString &mcpServerUuid);

private:
    explicit MCPService(QObject *parent = nullptr);
    MCPService(const MCPService &) = delete;
    MCPService &operator=(const MCPService &) = delete;
    QFuture<std::shared_ptr<MCPClient>> _initClient(const QString &mcpServerUuid);

private:
    QMap<QString, std::shared_ptr<MCPClient>> m_clients;
    QMap<QString, QFuture<std::shared_ptr<MCPClient>>> m_pendingClients;
};

#endif // MCPSERVICE_H