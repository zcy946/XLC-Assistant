#ifndef MCPSERVICE_H
#define MCPSERVICE_H

#include <QObject>
#include <QHash>
#include <memory>
#include <mcp_sse_client.h>
#include <mcp_stdio_client.h>
#include <QFuture>
#include "DataManager.h"

struct MCPTool
{
    std::string id;          // 新生成的唯一id(将此id作为function name传给llm)
    std::string name;        // 工具原有的名字
    std::string serverUuid;  // 所在mcp服务器uuid
    mcp::json convertedTool; // 转换为json的工具
    MCPTool(const std::string &name, const std::string &serverUuid, const mcp::json &convertedTool);

private:
    std::string buildFunctionCallToolName(const std::string &serverUuid, const std::string &toolName);
};

struct MCPClient
{
    std::unique_ptr<mcp::client> client;
    QVector<MCPTool> tools; // 缓存该服务器提供的工具列表
};

struct CallToolArgs
{
    QString conversationUuid; // 对话uuid
    QString callId;           // 本次本次调用的id
    QString toolName;         // 调用的函数名称
    mcp::json parameters;     // 参数
};

class MCPService : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void sig_clientReady(const QString &serverUuid, std::shared_ptr<MCPClient> client);
    void sig_clientError(const QString &serverUuid, const QString &errorMessage);

public:
    /**
     *
        this.listPrompts = this.listPrompts.bind(this)
        this.getPrompt = this.getPrompt.bind(this)
        this.listResources = this.listResources.bind(this)
        this.getResource = this.getResource.bind(this)
        checkMcpConnectivity;
     */
    static MCPService *getInstance();
    ~MCPService() = default;
    void initClient(const QString &serverUuid);
    void closeClient(const QString &serverUuid);
    void callTool(const CallToolArgs &callToolArgs);

private:
    explicit MCPService(QObject *parent = nullptr);
    MCPService(const MCPService &) = delete;
    MCPService &operator=(const MCPService &) = delete;
    std::shared_ptr<MCPClient> createStdioClient(std::shared_ptr<McpServer> server);
    std::shared_ptr<MCPClient> createSSEClient(std::shared_ptr<McpServer> server);
    std::shared_ptr<MCPClient> createMCPClient(const QString &serverUuid);
    QVector<MCPTool> getTools(const std::string &serverUuid, mcp::client *client);

private:
    static MCPService *s_instance;
    QHash<QString, std::shared_ptr<MCPClient>> m_clients;                 // 服务器uuid - mcpClient
    QHash<QString, QFuture<std::shared_ptr<MCPClient>>> m_pendingClients; // 服务器uuid - QFuture<mcpClient>
    QHash<QString, std::shared_ptr<MCPClient>> m_tools;                   // MCPTool唯一id - mcpClient
};

#endif // MCPSERVICE_H