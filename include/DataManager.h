#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <memory>
#include <QHash>
#include <global.h>
#include <QObject>
#include "Logger.hpp"
// #include <mcp_message.h>
#include <QJsonArray>
#include <QSet>
#include <QMutex>
#include "DataBaseManager.h"

struct CallToolArgs;
struct LLM;
struct McpServer;
struct Agent;
struct Message;
struct Conversation;
class DataManager : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void sig_LLMsLoaded(bool success);
    void sig_mcpServersLoaded(bool success);
    void sig_agentsLoaded(bool success);
    void sig_messagesLoaded(const QString &conversationUuid);
    void sig_conversationsLoaded(bool success);
    void sig_LLMsFilePathChange(const QString &filePath);
    void sig_mcpServersFilePathChange(const QString &filePath);
    void sig_agentsFilePathChange(const QString &filePath);
    void sig_LLMUpdate(const QString &uuid);
    void sig_mcpUpdate(const QString &uuid);
    void sig_agentUpdate(const QString &uuid);

private Q_SLOTS:
    // 处理 MCP Servers 加载完毕事件
    void slot_onMcpServersLoaded(bool success);
    // 处理从数据库获取到所有Conversation数据事件
    void slot_handleAllConversationInfoAcquired(bool success, QJsonArray jsonArrayConversations);
    // 处理从数据库获取到消息列表事件
    void slot_handleMessagesAcquired(bool success, const QString &conversationUuid, QJsonArray jsonArrayMessages);

public:
    static DataManager *getInstance();
    ~DataManager() = default;
    static void registerAllMetaType();
    void init();

    // LLM
    bool loadLLMs(const QString &filePath);
    void addLLM(std::shared_ptr<LLM> llm);
    void removeLLM(const QString &uuid);
    void updateLLM(std::shared_ptr<LLM> llm);
    void saveLLMs(const QString &filePath) const;
    void saveLLMsAsync(const QString &filePath = QString()) const;
    std::shared_ptr<LLM> getLLM(const QString &uuid) const;
    QList<std::shared_ptr<LLM>> getLLMs() const;
    void setFilePathLLMs(const QString &filePath);
    const QString &getFilePathLLMs() const;
    // MCP Server
    bool loadMcpServers(const QString &filePath);
    void addMcpServer(std::shared_ptr<McpServer> mcpServer);
    void removeMcpServer(const QString &uuid);
    void updateMcpServer(std::shared_ptr<McpServer> mcpServer);
    void saveMcpServers(const QString &filePath) const;
    void saveMcpServersAsync(const QString &filePath = QString()) const;
    std::shared_ptr<McpServer> getMcpServer(const QString &uuid) const;
    QList<std::shared_ptr<McpServer>> getMcpServers() const;
    void setFilePathMcpServers(const QString &filePath);
    const QString &getFilePathMcpServers() const;
    // Agent
    bool loadAgents(const QString &filePath);
    void removeAgent(const QString &uuid);
    void updateAgent(std::shared_ptr<Agent> newAgent);
    void saveAgents(const QString &filePath) const;
    void saveAgentsAsync(const QString &filePath = QString()) const;
    void addAgent(std::shared_ptr<Agent> agent);
    std::shared_ptr<Agent> getAgent(const QString &uuid) const;
    QList<std::shared_ptr<Agent>> getAgents() const;
    void setFilePathAgents(const QString &filePath);
    const QString &getFilePathAgents() const;
    // conversation
    bool loadConversations();
    // 添加对话，并同步到配置文件
    void addConversation(std::shared_ptr<Conversation> conversation);
    void removeConversation(const QString &uuid);
    void updateConversation(std::shared_ptr<Conversation> newConversation);
    std::shared_ptr<Conversation> getConversation(const QString &uuid) const;
    QList<std::shared_ptr<Conversation>> getConversations() const;
    // 通过`agentUuid`创建新的`conversation`并绑定到该`agent`
    std::shared_ptr<Conversation> createNewConversation(const QString &agentUuid);
    // 添加对话到正在获取消息列表
    void addPengingConversation(const QString &conversationUuid);
    // 查看是否正在获取消息
    bool isPendingConversations(const QString &conversationUuid);
    // 从正在获取消息列表中删除对话
    void removePengingConversation(const QString &conversationUuid);

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
    QSet<QString> m_pendingConversations;                          // conversationUuid - 正在从数据库获取消息的对话
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

struct Agent
{
    QString uuid;        // 唯一标识符
    QString name;        // 名字
    QString description; // 描述
    // llm参数
    int context;                 // 上下文数量
    QString systemPrompt;        // 系统提示词
    QString llmUUid;             // LLM ID
    double temperature;          // 文档
    double topP;                 // top-P
    int maxTokens;               // 最大tokens
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

struct Conversation : public std::enable_shared_from_this<Conversation>
{
protected:
    Conversation(const QString &agentUuid);
    Conversation(const QString &uuid,
                 const QString &agentUuid,
                 const QString &summary,
                 const QString &createdTime,
                 const QString &updatedTime,
                 int messageCount);

public:
    static std::shared_ptr<Conversation> create(const QString &agentUuid);
    static std::shared_ptr<Conversation> create(const QString &uuid,
                                                const QString &agentUuid,
                                                const QString &summary,
                                                const QString &createdTime,
                                                const QString &updatedTime,
                                                int messageCount);

    // 是否有系统提示词
    bool hasSystemPrompt();
    // 重设系统提示词
    void resetSystemPrompt();
    // 添加消息
    void addMessage(const Message &newMessage);
    // 获取所有消息
    const QVector<Message> getMessages();
    // 获取json格式的messages
    const QJsonArray getCachedMessages();
    // 清除上下文
    void clearContext();
    // 从[数据库获取的]消息列表中加载
    void loadMessages(const QList<Message> &messages);

public:
    QString uuid;             // 对话唯一标识
    QString agentUuid;        // agent唯一标识
    QString summary;          // 摘要-对话标题
    QString createdTime;      // 创建时间
    QString updatedTime;      // 更新时间
    int messageCount = -1;    // 记录消息数量(非缓存消息)，初始化为 -1 代表未同步/同步失败数据库
    int pendingToolCalls = 0; // 待处理的工具调用数量

private:
    QMutex mutex_jsonArrayCachedMessages;
    QJsonArray jsonArrayCachedMessages;
    QVector<Message> messages;
};

struct Message
{
    QString id;
    QString content;
    enum Role
    {
        USER = 0,
        ASSISTANT = 1,
        TOOL = 2,
        SYSTEM = 3,
        UNKNOWN = 4
    };
    Role role;
    QString createdTime;
    QJsonArray toolCalls;
    QString toolCallId;
    QString avatarFilePath;

    Message(const QString &id,
            const QString &content,
            Role role,
            const QString &createdTime,
            const QJsonArray &toolCalls,
            const QString &toolCallId,
            const QString &avatarFilePath)
        : id(id), content(content), role(role), createdTime(createdTime), toolCalls(toolCalls), toolCallId(toolCallId), avatarFilePath(avatarFilePath)
    {
    }

    Message(const QString &content,
            Role role,
            const QString &createdTime,
            const QJsonArray &toolCalls = QJsonArray(),
            const QString &toolCallId = QString())
        : id(generateUuid()), content(content), role(role), createdTime(createdTime), toolCalls(toolCalls), toolCallId(toolCallId)
    {
        if (this->avatarFilePath.isEmpty())
        {
            switch (role)
            {
            case Role::USER:
                this->avatarFilePath = QString(DEFAULT_AVATAR_USER);
                break;
            case Role::ASSISTANT:
                this->avatarFilePath = QString(DEFAULT_AVATAR_LLM);
                break;
            case Role::TOOL:
                this->avatarFilePath = QString(AVATAR_TOOL);
                break;
            case Role::SYSTEM:
                this->avatarFilePath = QString(AVATAR_SYSTEM);
                break;
            default:
                this->avatarFilePath = QString(AVATAR_UNKNOW);
                break;
            }
        }
    }
};

#endif // DATAMANAGER_H