#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include "Singleton.h"
#include <memory>
#include <QHash>
#include <global.h>

struct McpServer;
struct Agent;
struct Conversation;
class DataManager : public Singleton<DataManager>
{
    friend class Singleton<DataManager>;

public:
    ~DataManager() = default;
    static void registerAllMetaType();
    void init();

    /**
     * 加载所有mcp服务器
     */
    bool loadMcpServers(const QString &filePath);

    /**
     * 新增mcp服务器
     */
    void addMcpServer(const std::shared_ptr<McpServer> &mcpServer);

    /**
     * 通过uuid删除mcp服务器
     */
    void removeMcpServer(const QString &uuid);

    /**
     * 更新mcp服务器
     */
    void updateMcpServer(const McpServer &mcpServer);

    /**
     * 保存所有mcp服务器到文件
     */
    void saveMcpServers(const QString &filePath) const;

    /**
     * 通过uuid获取mcp服务器
     */
    std::shared_ptr<McpServer> getMcpServer(const QString &uuid) const;

    /**
     * 获取所有mcp服务器
     */
    QList<std::shared_ptr<McpServer>> getMcpServers() const;

    /**
     * 加载所有agent
     */
    bool loadAgents(const QString &filePath);

    /**
     * 通过uuid删除agent
     */
    void removeAgent(const QString &uuid);

    /**
     * 更新agent
     */
    void updateAgent(const Agent &agent);

    /**
     * 保存所有agent到文件
     */
    void saveAgents(const QString &filePath) const;

    /**
     * 新增agent
     */
    void addAgent(const std::shared_ptr<Agent> &agent);

    /**
     * 通过uuid获取agent
     */
    std::shared_ptr<Agent> getAgent(const QString &uuid) const;

    /**
     * 获取所有agent
     */
    QList<std::shared_ptr<Agent>> getAgents() const;

    /**
     * 加载所有对话
     */
    void loadConversations();

    /**
     * 新增对话
     */
    void addConversation(const std::shared_ptr<Conversation> &conversation);

    /**
     * 通过uuid删除Conversation
     */
    void removeConversation(const QString &uuid);

    /**
     * 更新Conversation
     */
    void updateConversation(const Conversation &conversation);

    /**
     * 通过uuid获取对话
     */
    std::shared_ptr<Conversation> getConversation(const QString &uuid) const;

    /**
     * 获取所有对话
     */
    QList<std::shared_ptr<Conversation>> getConversations() const;

    /**
     * 设置mcp服务器文件路径
     */
    void setFilePathMcpServers(const QString &filePath);

    /**
     * 获取mcp服务器文件路径
     */
    const QString &getFilePathMcpServers(const QString &filePath) const;

    /**
     * 设置agents文件路径
     */
    void setFilePathAgents(const QString &filePath);

    /**
     * 获取agents文件路径
     */
    const QString &getFilePathAgents(const QString &filePath) const;

private:
    DataManager();

private:
    QString m_filePathMcpServers;
    QString m_filePathAgents;
    QHash<QString, std::shared_ptr<McpServer>> m_mcpServers;
    QHash<QString, std::shared_ptr<Agent>> m_agents;
    QHash<QString, std::shared_ptr<Conversation>> m_conversations;
};

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
    QString url;
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
          url(),
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
              const QString &url,
              const QString &requestHeaders)
        : uuid(generateUuid()),
          name(name),
          description(description),
          type(type),
          timeout(timeout),
          command(command),
          args(args),
          envVars(envVars),
          url(url),
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
            server.url = jsonObject["url"].toString();
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
            jsonObject["url"] = url;
            jsonObject["requestHeaders"] = requestHeaders;
        }
        return jsonObject;
    }
};
Q_DECLARE_METATYPE(McpServer)

struct Agent
{
    QString uuid;
    QString name;
    QString description;
    int children; // 以该agent为模板的对话数量
    // llm参数
    int context;
    QString systemPrompt;
    QString modelName;
    double temperature;
    double topP;
    int maxTokens;
    QVector<McpServer> mcpServers;

    Agent()
        : name(),
          description(),
          children(),
          context(),
          systemPrompt(),
          modelName(),
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
          const QString &modelName,
          double temperature,
          double topP,
          int maxTokens,
          const QVector<McpServer> &mcpServers = QVector<McpServer>())
        : uuid(generateUuid()),
          name(name),
          description(description),
          children(children),
          context(context),
          systemPrompt(systemPrompt),
          modelName(modelName),
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
        agent.modelName = jsonObject["modelName"].toString();
        agent.temperature = jsonObject["temperature"].toDouble();
        agent.topP = jsonObject["topP"].toDouble();
        agent.maxTokens = jsonObject["maxTokens"].toInt();

        QJsonArray mcpServersArray = jsonObject["mcpServers"].toArray();
        for (const QJsonValue &value : mcpServersArray)
        {
            if (value.isObject())
            {
                agent.mcpServers.append(McpServer::fromJson(value.toObject()));
            }
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
        jsonObject["modelName"] = modelName;
        jsonObject["temperature"] = temperature;
        jsonObject["topP"] = topP;
        jsonObject["maxTokens"] = maxTokens;

        QJsonArray mcpServersArray;
        for (const McpServer &server : mcpServers)
        {
            mcpServersArray.append(server.toJsonObject());
        }
        jsonObject["mcpServers"] = mcpServersArray;
        return jsonObject;
    }
};
Q_DECLARE_METATYPE(Agent)

struct Conversation
{
    QString uuid;
    QString summary; // 对话摘要
    QDateTime createdTime;
    QDateTime updatedTime;

    Conversation()
        : uuid(generateUuid()),
          summary(),
          createdTime(QDateTime::currentDateTime()),
          updatedTime(QDateTime::currentDateTime())
    {
    }
    Conversation(const QString &summary,
                 const QDateTime &createdTime,
                 const QDateTime &updatedTime)
        : uuid(generateUuid()),
          summary(summary),
          createdTime(createdTime),
          updatedTime(updatedTime)
    {
    }
};
Q_DECLARE_METATYPE(Conversation)

#endif // DATAMANAGER_H