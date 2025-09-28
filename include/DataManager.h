#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <memory>
#include <QHash>
#include <global.h>
#include <QObject>
#include "Logger.hpp"
#include <mcp_message.h>
#include <QSet>
#include <QMutex>
#include "DataBaseManager.h"

struct CallToolArgs;
struct LLM;
struct McpServer;
struct Agent;
struct Conversation;
class DataManager : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void sig_LLMsLoaded(bool success);
    void sig_mcpServersLoaded(bool success);
    void sig_agentsLoaded(bool success);
    void sig_conversationsLoaded(bool success);
    void sig_LLMsFilePathChange(const QString &filePath);
    void sig_mcpServersFilePathChange(const QString &filePath);
    void sig_agentsFilePathChange(const QString &filePath);
    void sig_LLMUpdate(const QString &uuid);
    void sig_mcpUpdate(const QString &uuid);
    void sig_agentUpdate(const QString &uuid);

private Q_SLOTS:
    void slot_onMcpServersLoaded(bool success);

public:
    static DataManager *getInstance();
    ~DataManager() = default;
    static void registerAllMetaType();
    void init();

    bool loadLLMs(const QString &filePath);
    void addLLM(const std::shared_ptr<LLM> &llm);
    void removeLLM(const QString &uuid);
    void updateLLM(const std::shared_ptr<LLM> &llm);
    void saveLLMs(const QString &filePath) const;
    void saveLLMsAsync(const QString &filePath) const;
    std::shared_ptr<LLM> getLLM(const QString &uuid) const;
    QList<std::shared_ptr<LLM>> getLLMs() const;
    void setFilePathLLMs(const QString &filePath);
    const QString &getFilePathLLMs() const;

    bool loadMcpServers(const QString &filePath);
    void addMcpServer(const std::shared_ptr<McpServer> &mcpServer);
    void removeMcpServer(const QString &uuid);
    void updateMcpServer(const std::shared_ptr<McpServer> &mcpServer);
    void saveMcpServers(const QString &filePath) const;
    void saveMcpServersAsync(const QString &filePath) const;
    std::shared_ptr<McpServer> getMcpServer(const QString &uuid) const;
    QList<std::shared_ptr<McpServer>> getMcpServers() const;
    void setFilePathMcpServers(const QString &filePath);
    const QString &getFilePathMcpServers() const;

    bool loadAgents(const QString &filePath);
    void removeAgent(const QString &uuid);
    void updateAgent(const std::shared_ptr<Agent> &newAgent);
    void saveAgents(const QString &filePath) const;
    void saveAgentsAsync(const QString &filePath) const;
    void addAgent(const std::shared_ptr<Agent> &agent);
    std::shared_ptr<Agent> getAgent(const QString &uuid) const;
    QList<std::shared_ptr<Agent>> getAgents() const;
    void setFilePathAgents(const QString &filePath);
    const QString &getFilePathAgents() const;

    bool loadConversations();
    void addConversation(const std::shared_ptr<Conversation> &conversation);
    void removeConversation(const QString &uuid);
    void updateConversation(std::shared_ptr<Conversation> newConversation);
    std::shared_ptr<Conversation> getConversation(const QString &uuid) const;
    QList<std::shared_ptr<Conversation>> getConversations() const;
    std::shared_ptr<Conversation> createNewConversation(const QString &agentUuid);

private:
    explicit DataManager(QObject *parent = nullptr);
    DataManager(const DataManager &) = delete;
    DataManager &operator=(const DataManager &) = delete;
    void loadDataAsync();
    void loadLLMsAsync();
    void loadMcpServersAsync();
    void loadAgentsAsync();

private:
    static DataManager *s_instance;
    QString m_filePathLLMs;
    QString m_filePathMcpServers;
    QString m_filePathAgents;
    QHash<QString, std::shared_ptr<LLM>> m_llms;                   // uuid - ptr
    QHash<QString, std::shared_ptr<McpServer>> m_mcpServers;       // uuid - ptr
    QHash<QString, std::shared_ptr<Agent>> m_agents;               // uuid - ptr
    QHash<QString, std::shared_ptr<Conversation>> m_conversations; // uuid - ptr
};

struct LLM
{
    QString uuid;
    QString modelID;
    QString modelName;
    QString apiKey;
    QString baseUrl;
    QString endpoint;

    LLM();
    LLM(const QString &modelID,
        const QString &modelName,
        const QString &apiKey,
        const QString &baseUrl,
        const QString &endpoint = QString());
    static LLM fromJson(const QJsonObject &jsonObject);

    QJsonObject toJsonObject() const;
};
// Q_DECLARE_METATYPE(LLM)

struct McpServer
{
    enum Type
    {
        stdio = 0,
        sse = 1,
        streambleHttp = 2
    };
    bool isActive;
    QString uuid;
    QString name;
    QString description;
    Type type;
    int timeout; // 单位: 秒(s)
    // stdio参数
    QString command;
    QVector<QString> args;
    QMap<QString, QString> envVars;
    // sse&streambleHttp参数
    QString host;
    int port;
    QString baseUrl;
    QString endpoint;
    QString requestHeaders;

    McpServer();
    McpServer(bool isActive,
              const QString &name,
              const QString &description,
              Type type,
              int timeout,
              const QString &command,
              const QVector<QString> &args,
              const QMap<QString, QString> &envVars,
              const QString &baseUrl,
              const QString &endpoint,
              const QString &requestHeaders);
    McpServer(bool isActive,
              const QString &name,
              const QString &description,
              Type type,
              int timeout,
              const QString &command,
              const QVector<QString> &args,
              const QMap<QString, QString> &envVars,
              const QString &host,
              int port,
              const QString &endpoint,
              const QString &requestHeaders);
    // 从 QJsonObject 解析 McpServer
    static McpServer fromJson(const QJsonObject &jsonObject);
    // 将 McpServer 序列化为 QJsonObject
    QJsonObject toJsonObject() const;
};
// Q_DECLARE_METATYPE(McpServer)

struct Agent
{
    QString uuid;
    QString name;
    QString description;
    // llm参数
    int context;
    QString systemPrompt;
    QString llmUUid;
    double temperature;
    double topP;
    int maxTokens;
    QSet<QString> mcpServers;    // 挂载的mcp服务器的uuid
    QSet<QString> conversations; // 使用该agent的对话的uuid

    Agent();
    Agent(const QString &name,
          const QString &description,
          int context,
          const QString &systemPrompt,
          const QString &llmUUid,
          double temperature,
          double topP,
          int maxTokens,
          const QSet<QString> &mcpServers = QSet<QString>(),
          const QSet<QString> &conversations = QSet<QString>());
    // 从 QJsonObject 解析 Agent
    static Agent fromJson(const QJsonObject &jsonObject);
    // 将 Agent 序列化为 QJsonObject
    QJsonObject toJsonObject() const;
};
// Q_DECLARE_METATYPE(Agent)

struct Conversation : public std::enable_shared_from_this<Conversation>
{
    QString uuid;
    QString agentUuid;
    QString summary;
    QDateTime createdTime;
    QDateTime updatedTime;
    int messageCount; // 记录消息数量

private:
    QMutex mutex;
    mcp::json messages;

public:
    static std::shared_ptr<Conversation> create(const QString &agentUuid);
    static std::shared_ptr<Conversation> create(const QString &uuid,
                                                const QString &agentUuid,
                                                const QString &summary,
                                                const QDateTime &createdTime,
                                                const QDateTime &updatedTime);
    bool hasSystemPrompt();
    void resetSystemPrompt();
    void addMessage(const mcp::json &newMessage);
    const mcp::json getMessages();
    // 清除上下文
    void clearContext();

protected:
    Conversation(const QString &agentUuid);
    Conversation(const QString &uuid,
                 const QString &agentUuid,
                 const QString &summary,
                 const QDateTime &createdTime,
                 const QDateTime &updatedTime);
};

// Q_DECLARE_METATYPE(Conversation)

#endif // DATAMANAGER_H