#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include "Singleton.h"
#include <memory>
#include <QHash>
#include <global.h>

class DataManager : public Singleton<DataManager>
{
    friend class Singleton<DataManager>;

public:
    ~DataManager() = default;

    /**
     * 加载所有mcp服务器
     */
    void loadMcpServers();

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
    void loadAgents();

    /**
     * 通过uuid删除agent
     */
    void removeAgent(const QString &uuid);

    /**
     * 更新agent
     */
    void updateAgent(const Agent &agent);

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

private:
    DataManager();

private:
    QHash<QString, std::shared_ptr<McpServer>> m_mcpServers;
    QHash<QString, std::shared_ptr<Agent>> m_agents;
    QHash<QString, std::shared_ptr<Conversation>> m_conversations;
};

#endif // DATAMANAGER_H