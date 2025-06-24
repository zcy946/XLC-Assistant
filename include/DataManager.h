#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <memory>
#include <QHash>
#include <global.h>
#include <QObject>
#include "Logger.hpp"
#include <mcp_message.h>
#include <QSet>
#include "MCPGateWay.h"

class LLMService;
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
    void sig_filePathChangedLLMs(const QString &filePath);
    void sig_filePathChangedMcpServers(const QString &filePath);
    void sig_filePathChangedAgents(const QString &filePath);

private Q_SLOTS:
    void slot_onMcpServersLoaded(bool success);
    void slot_onResponseReady(const QString &conversationUuid, const QString &responseJson);
    void slot_onToolCallSucceeded(const QString &conversationUuid, const QString &callId, const QString &toolName, const QString &resultJson);
    void slot_onToolCallFailed(const QString &conversationUuid, const QString &callId, const QString &toolName, const QString &error);

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

    // 处理用户发送消息事件
    void handleMessageSent(const std::shared_ptr<Conversation> &conversation, const std::shared_ptr<Agent> &agent, const mcp::json &tools, int max_retries = 3);
    const mcp::json getTools(const QSet<QString> mcpServers);

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
    QHash<QString, std::shared_ptr<LLM>> m_llms;
    QHash<QString, std::shared_ptr<McpServer>> m_mcpServers;
    QHash<QString, std::shared_ptr<Agent>> m_agents;
    QHash<QString, std::shared_ptr<Conversation>> m_conversations;
    LLMService *m_llmService;
    McpGateway *m_mcpGatway;
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
          timeout(60),
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
    int children; // 以该agent为模板的对话数量
    // llm参数
    int context;
    QString systemPrompt;
    QString llmUUid;
    double temperature;
    double topP;
    int maxTokens;
    QSet<QString> mcpServers; // 挂载的mcp服务器的uuid
    // TODO 添加 QSet<QString>conversations 以该agent为模板的对话的uuid

    Agent()
        : name(),
          description(),
          children(),
          context(),
          systemPrompt(),
          llmUUid(),
          temperature(),
          topP(),
          maxTokens(),
          mcpServers()
    {
    }
    Agent(const QString &name,
          const QString &description,
          int children,
          int context,
          const QString &systemPrompt,
          const QString &llmUUid,
          double temperature,
          double topP,
          int maxTokens,
          const QSet<QString> &mcpServers = QSet<QString>())
        : uuid(generateUuid()),
          name(name),
          description(description),
          children(children),
          context(context),
          systemPrompt(systemPrompt),
          llmUUid(llmUUid),
          temperature(temperature),
          topP(topP),
          maxTokens(maxTokens),
          mcpServers(mcpServers)
    {
    }

    // 从 QJsonObject 解析 Agent
    static Agent fromJson(const QJsonObject &jsonObject)
    {
        Agent agent;
        agent.uuid = jsonObject["uuid"].toString();
        agent.name = jsonObject["name"].toString();
        agent.description = jsonObject["description"].toString();
        agent.children = jsonObject["children"].toInt();
        agent.context = jsonObject["context"].toInt();
        agent.systemPrompt = jsonObject["systemPrompt"].toString();
        agent.llmUUid = jsonObject["llmUUid"].toString();
        agent.temperature = jsonObject["temperature"].toDouble();
        agent.topP = jsonObject["topP"].toDouble();
        agent.maxTokens = jsonObject["maxTokens"].toInt();

        QJsonArray mcpServersArray = jsonObject["mcpServers"].toArray();
        for (const QJsonValue &value : mcpServersArray)
        {
            agent.mcpServers.insert(value.toString());
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
        jsonObject["children"] = children;
        jsonObject["context"] = context;
        jsonObject["systemPrompt"] = systemPrompt;
        jsonObject["llmUUid"] = llmUUid;
        jsonObject["temperature"] = temperature;
        jsonObject["topP"] = topP;
        jsonObject["maxTokens"] = maxTokens;

        QJsonArray mcpServersArray;
        for (const QString &serverUuid : mcpServers)
        {
            mcpServersArray.append(serverUuid);
        }
        jsonObject["mcpServers"] = mcpServersArray;
        return jsonObject;
    }
};
// Q_DECLARE_METATYPE(Agent)

struct Conversation
{
    QString uuid;
    QString summary; // 对话摘要
    QDateTime createdTime;
    QDateTime updatedTime;
    mcp::json messages;

    Conversation()
        : uuid(generateUuid()),
          summary(),
          createdTime(QDateTime::currentDateTime()),
          updatedTime(QDateTime::currentDateTime()),
          messages(mcp::json())
    {
    }
    Conversation(const QString &summary,
                 const QDateTime &createdTime,
                 const QDateTime &updatedTime)
        : uuid(generateUuid()),
          summary(summary),
          createdTime(createdTime),
          updatedTime(updatedTime),
          messages(mcp::json())
    {
    }
};
// Q_DECLARE_METATYPE(Conversation)

#endif // DATAMANAGER_H