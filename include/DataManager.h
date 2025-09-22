#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <memory>
#include <QHash>
#include <global.h>
#include <QObject>
#include "Logger.hpp"
#include <mcp_message.h>
#include <QSet>

class LLMService;
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
    void slot_onToolCallFinished(const CallToolArgs &callToolArgs, bool success, const mcp::json &result, const QString &errorMessage);

public:
    static DataManager *getInstance();
    ~DataManager() = default;
    // static void registerAllMetaType();
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
    void updateAgent(const std::shared_ptr<Agent> &agent);
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
    void updateConversation(const Conversation &conversation);
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

    LLM()
        : uuid(generateUuid()),
          modelID(),
          modelName(),
          apiKey(),
          baseUrl(),
          endpoint("/v1/chat/completions")
    {
    }

    LLM(const QString &modelID,
        const QString &modelName,
        const QString &apiKey,
        const QString &baseUrl,
        const QString &endpoint = QString())
        : uuid(generateUuid()),
          modelID(modelID),
          modelName(modelName),
          apiKey(apiKey),
          baseUrl(baseUrl),
          endpoint(endpoint)
    {
    }

    static LLM fromJson(const QJsonObject &jsonObject)
    {
        LLM llm;
        llm.uuid = jsonObject["uuid"].toString();
        llm.modelID = jsonObject["modelID"].toString();
        llm.modelName = jsonObject["modelName"].toString();
        llm.apiKey = jsonObject["apiKey"].toString();
        llm.baseUrl = jsonObject["baseUrl"].toString();
        llm.endpoint = jsonObject["endpoint"].toString();
        return llm;
    }

    QJsonObject toJsonObject() const
    {
        QJsonObject jsonObject;
        jsonObject["uuid"] = uuid;
        jsonObject["modelID"] = modelID;
        jsonObject["modelName"] = modelName;
        jsonObject["apiKey"] = apiKey;
        jsonObject["baseUrl"] = baseUrl;
        jsonObject["endpoint"] = endpoint;
        return jsonObject;
    }
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

    McpServer()
        : uuid(generateUuid()),
          name(),
          description(),
          type(sse),
          timeout(30),
          command(),
          args(),
          envVars(),
          host(),
          port(),
          baseUrl(),
          endpoint("/sse"),
          requestHeaders()
    {
    }
    McpServer(const QString &name,
              const QString &description,
              Type type,
              int timeout,
              const QString &command,
              const QVector<QString> &args,
              const QMap<QString, QString> &envVars,
              const QString &baseUrl,
              const QString &endpoint,
              const QString &requestHeaders)
        : uuid(generateUuid()),
          name(name),
          description(description),
          type(type),
          timeout(timeout),
          command(command),
          args(args),
          envVars(envVars),
          host(),
          port(),
          baseUrl(baseUrl),
          endpoint(endpoint),
          requestHeaders(requestHeaders)
    {
    }
    McpServer(const QString &name,
              const QString &description,
              Type type,
              int timeout,
              const QString &command,
              const QVector<QString> &args,
              const QMap<QString, QString> &envVars,
              const QString &host,
              int port,
              const QString &endpoint,
              const QString &requestHeaders)
        : uuid(generateUuid()),
          name(name),
          description(description),
          type(type),
          timeout(timeout),
          command(command),
          args(args),
          envVars(envVars),
          host(host),
          port(port),
          baseUrl(),
          endpoint(endpoint),
          requestHeaders(requestHeaders)
    {
    }

    // 从 QJsonObject 解析 McpServer
    static McpServer fromJson(const QJsonObject &jsonObject)
    {
        McpServer server;
        server.uuid = jsonObject["uuid"].toString();
        server.name = jsonObject["name"].toString();
        server.description = jsonObject["description"].toString();
        server.type = static_cast<Type>(jsonObject["type"].toInt());
        server.timeout = jsonObject["timeout"].toInt();

        if (server.type == stdio)
        {
            server.command = jsonObject["command"].toString();
            QJsonArray argsArray = jsonObject["args"].toArray();
            for (const QJsonValue &value : argsArray)
            {
                server.args.append(value.toString());
            }

            QJsonObject envVarsObject = jsonObject["envVars"].toObject();
            for (auto it = envVarsObject.begin(); it != envVarsObject.end(); ++it)
            {
                server.envVars.insert(it.key(), it.value().toString());
            }
        }
        else if (server.type == sse || server.type == streambleHttp)
        {
            server.host = jsonObject["host"].toString();
            server.port = jsonObject["port"].toInt();
            server.baseUrl = jsonObject["baseUrl"].toString();
            server.endpoint = jsonObject["endpoint"].toString();
            server.requestHeaders = jsonObject["requestHeaders"].toString();
        }
        return server;
    }

    // 将 McpServer 序列化为 QJsonObject
    QJsonObject toJsonObject() const
    {
        QJsonObject jsonObject;
        jsonObject["uuid"] = uuid;
        jsonObject["name"] = name;
        jsonObject["description"] = description;
        jsonObject["type"] = type;
        jsonObject["timeout"] = timeout;

        if (type == stdio)
        {
            jsonObject["command"] = command;
            QJsonArray argsArray;
            for (const QString &arg : args)
            {
                argsArray.append(arg);
            }
            jsonObject["args"] = argsArray;

            QJsonObject envVarsObject;
            for (auto it = envVars.begin(); it != envVars.end(); ++it)
            {
                envVarsObject.insert(it.key(), it.value());
            }
            jsonObject["envVars"] = envVarsObject;
        }
        else if (type == sse || type == streambleHttp)
        {
            jsonObject["host"] = host;
            jsonObject["port"] = port;
            jsonObject["baseUrl"] = baseUrl;
            jsonObject["endpoint"] = endpoint;
            jsonObject["requestHeaders"] = requestHeaders;
        }
        return jsonObject;
    }
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

    Agent()
        : name(),
          description(),
          context(),
          systemPrompt(),
          llmUUid(),
          temperature(),
          topP(),
          maxTokens(),
          mcpServers(),
          conversations()
    {
    }
    Agent(const QString &name,
          const QString &description,
          int context,
          const QString &systemPrompt,
          const QString &llmUUid,
          double temperature,
          double topP,
          int maxTokens,
          const QSet<QString> &mcpServers = QSet<QString>(),
          const QSet<QString> &conversations = QSet<QString>())
        : uuid(generateUuid()),
          name(name),
          description(description),
          context(context),
          systemPrompt(systemPrompt),
          llmUUid(llmUUid),
          temperature(temperature),
          topP(topP),
          maxTokens(maxTokens),
          mcpServers(mcpServers),
          conversations(conversations)
    {
    }

    // 从 QJsonObject 解析 Agent
    static Agent fromJson(const QJsonObject &jsonObject)
    {
        Agent agent;
        agent.uuid = jsonObject["uuid"].toString();
        agent.name = jsonObject["name"].toString();
        agent.description = jsonObject["description"].toString();
        agent.context = jsonObject["context"].toInt();
        agent.systemPrompt = jsonObject["systemPrompt"].toString();
        agent.llmUUid = jsonObject["llmUUid"].toString();
        agent.temperature = jsonObject["temperature"].toDouble();
        agent.topP = jsonObject["topP"].toDouble();
        agent.maxTokens = jsonObject["maxTokens"].toInt();

        QJsonArray mcpServersArray = jsonObject["mcpServers"].toArray();
        for (const QJsonValue &mcpServerUuid : mcpServersArray)
        {
            agent.mcpServers.insert(mcpServerUuid.toString());
        }
        QJsonArray conversationsArray = jsonObject["conversations"].toArray();
        for (const QJsonValue &conversationUuid : conversationsArray)
        {
            agent.conversations.insert(conversationUuid.toString());
        }
        return agent;
    }

    // 将 Agent 序列化为 QJsonObject
    QJsonObject toJsonObject() const
    {
        QJsonObject jsonObject;
        jsonObject["uuid"] = uuid;
        jsonObject["name"] = name;
        jsonObject["description"] = description;
        jsonObject["context"] = context;
        jsonObject["systemPrompt"] = systemPrompt;
        jsonObject["llmUUid"] = llmUUid;
        jsonObject["temperature"] = temperature;
        jsonObject["topP"] = topP;
        jsonObject["maxTokens"] = maxTokens;

        QJsonArray mcpServersArray;
        for (const QString &mcpServerUuid : mcpServers)
        {
            mcpServersArray.append(mcpServerUuid);
        }
        jsonObject["mcpServers"] = mcpServersArray;
        QJsonArray conversationsArray;
        for (const QString &conversationUuid : conversations)
        {
            conversationsArray.append(conversationUuid);
        }
        jsonObject["conversations"] = conversationsArray;
        return jsonObject;
    }
};
// Q_DECLARE_METATYPE(Agent)

struct Conversation : public std::enable_shared_from_this<Conversation>
{
    QString uuid;
    QString agentUuid;
    QString summary;
    QDateTime createdTime;
    QDateTime updatedTime;
    mcp::json messages;

    static std::shared_ptr<Conversation> create(const QString &agentUuid)
    {
        // 通过内部 enabler 来调用 protected ctor
        struct make_shared_enabler : public Conversation
        {
            make_shared_enabler(const QString &agentUuid)
                : Conversation(agentUuid) {}
            make_shared_enabler(const QString &uuid,
                                const QString &agentUuid,
                                const QString &summary,
                                const QDateTime &createdTime,
                                const QDateTime &updatedTime)
                : Conversation(uuid, agentUuid, summary, createdTime, updatedTime) {}
        };
        return std::static_pointer_cast<Conversation>(std::make_shared<make_shared_enabler>(agentUuid));
        /**
         * NOTE
         * 问题根源：std::make_shared 是外部函数，不能访问 protected 构造函数。
           解决办法：用 make_shared_enabler（内部派生类）作为“桥”，让它调用 protected 构造函数，然后把结果当成 shared_ptr<Conversation> 返回。
           效果：外部只能通过 Conversation::create(...) 来生成对象，既安全又保持 make_shared 的高效内存分配。 */
    }

    static std::shared_ptr<Conversation> create(const QString &uuid,
                                                const QString &agentUuid,
                                                const QString &summary,
                                                const QDateTime &createdTime,
                                                const QDateTime &updatedTime)
    {
        struct make_shared_enabler : public Conversation
        {
            make_shared_enabler(const QString &uuid,
                                const QString &agentUuid,
                                const QString &summary,
                                const QDateTime &createdTime,
                                const QDateTime &updatedTime)
                : Conversation(uuid, agentUuid, summary, createdTime, updatedTime) {}
        };

        return std::static_pointer_cast<Conversation>(std::make_shared<make_shared_enabler>(uuid, agentUuid, summary, createdTime, updatedTime));
    }

protected:
    Conversation(const QString &agentUuid)
        : uuid(generateUuid()),
          agentUuid(agentUuid),
          createdTime(QDateTime::currentDateTime()),
          updatedTime(QDateTime::currentDateTime()),
          messages(mcp::json()) {}

    Conversation(const QString &uuid,
                 const QString &agentUuid,
                 const QString &summary,
                 const QDateTime &createdTime,
                 const QDateTime &updatedTime)
        : uuid(uuid),
          agentUuid(agentUuid),
          summary(summary),
          createdTime(createdTime),
          updatedTime(updatedTime),
          messages(mcp::json()) {}
};

// Q_DECLARE_METATYPE(Conversation)

#endif // DATAMANAGER_H