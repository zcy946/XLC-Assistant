#ifndef MCPSERVICE_H
#define MCPSERVICE_H

#include <QObject>
#include <QMap>
#include <memory>
#include <mcp_sse_client.h>
#include <mcp_stdio_client.h>
#include <QFuture>
#include "DataManager.h"

struct MCPClient
{
    std::unique_ptr<mcp::client> client;
    mcp::json availableTools; // 缓存该服务器提供的工具列表
};

class MCPService : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void sig_clientReady(const QString &serverUuid, std::shared_ptr<MCPClient> client);
    void sig_clientError(const QString &serverUuid, const QString &errorMessage);

public:
    /**
     * this.listTools = this.listTools.bind(this)
        this.callTool = this.callTool.bind(this)
        this.listPrompts = this.listPrompts.bind(this)
        this.getPrompt = this.getPrompt.bind(this)
        this.listResources = this.listResources.bind(this)
        this.getResource = this.getResource.bind(this)
        this.closeClient = this.closeClient.bind(this)
        checkMcpConnectivity;
     */
    static MCPService *getInstance();
    ~MCPService() = default;
    void initClient(const QString &serverUuid);

private:
    explicit MCPService(QObject *parent = nullptr);
    MCPService(const MCPService &) = delete;
    MCPService &operator=(const MCPService &) = delete;
    std::shared_ptr<MCPClient> createStdioClient(std::shared_ptr<McpServer> server);
    std::shared_ptr<MCPClient> createSSEClient(std::shared_ptr<McpServer> server);
    std::shared_ptr<MCPClient> createMCPClient(const QString &serverUuid);

private:
    static MCPService *s_instance;
    QMap<QString, std::shared_ptr<MCPClient>> m_clients;
    QMap<QString, QFuture<std::shared_ptr<MCPClient>>> m_pendingClients;
};

#endif // MCPSERVICE_H