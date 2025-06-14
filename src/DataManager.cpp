#include "DataManager.h"
#include "Logger.hpp"

DataManager::DataManager()
{
}

void DataManager::loadMcpServers()
{
    // Do nothing for now, but if you load from persistence,
    // you'd populate m_mcpServers hash here.
    // Example:
    // for (const auto& loadedServer : someDataSource) {
    //     m_mcpServers.insert(loadedServer->uuid, loadedServer);
    // }
}

void DataManager::addMcpServer(const std::shared_ptr<McpServer> &mcpServer)
{
    if (mcpServer)
    {
        m_mcpServers.insert(mcpServer->uuid.trimmed(), mcpServer);
    }
    else
    {
        LOG_WARN("Attempted to add a null McpServer shared_ptr.");
    }
}

void DataManager::removeMcpServer(const QString &uuid)
{
    m_mcpServers.remove(uuid.trimmed());
}

void DataManager::updateMcpServer(const McpServer &mcpServer)
{
    auto it = m_mcpServers.find(mcpServer.uuid.trimmed());
    if (it != m_mcpServers.end())
    {
        // it.value() 返回 std::shared_ptr<McpServer>
        // *it.value() 解引用智能指针，得到 McpServer 对象的引用
        // 然后执行 McpServer 的 operator=
        (*it.value()) = mcpServer;
        LOG_DEBUG("Updated McpServer with UUID: {}", mcpServer.uuid);
    }
    else
    {
        // 如果找不到，根据业务需求处理：
        // 1. 什么也不做（当前行为）
        LOG_WARN("McpServer with UUID {} not found for update. No action taken.", mcpServer.uuid);
        // 2. 抛出异常
        // throw std::runtime_error("McpServer not found for update");
        // 3. 将其作为新项添加
        // LOG_INFO("McpServer with UUID {} not found for update, adding as new.", mcpServer.uuid);
        // addMcpServer(std::make_shared<McpServer>(mcpServer)); // 注意：这里需要构造一个新的shared_ptr
    }
}

std::shared_ptr<McpServer> DataManager::getMcpServer(const QString &uuid) const
{
    auto it = m_mcpServers.find(uuid.trimmed());
    if (it != m_mcpServers.end())
    {
        return it.value();
    }
    return nullptr;
}

QList<std::shared_ptr<McpServer>> DataManager::getMcpServers() const
{
    return m_mcpServers.values();
}

void DataManager::loadAgents()
{
    // Do nothing
}

void DataManager::addAgent(const std::shared_ptr<Agent> &agent)
{
    if (agent)
    {
        m_agents.insert(agent->uuid.trimmed(), agent);
    }
    else
    {
        LOG_WARN("Attempted to add a null Agent shared_ptr.");
    }
}

void DataManager::removeAgent(const QString &uuid)
{
    m_agents.remove(uuid.trimmed());
}

void DataManager::updateAgent(const Agent &agent)
{
    auto it = m_agents.find(agent.uuid.trimmed());
    if (it != m_agents.end())
    {
        (*it.value()) = agent;
        LOG_DEBUG("Updated Agent with UUID: {}", agent.uuid);
    }
    else
    {
        LOG_WARN("Agent with UUID {} not found for update. No action taken.", agent.uuid);
    }
}

std::shared_ptr<Agent> DataManager::getAgent(const QString &uuid) const
{
    auto it = m_agents.find(uuid.trimmed());
    if (it != m_agents.end())
    {
        return it.value();
    }
    return nullptr;
}

QList<std::shared_ptr<Agent>> DataManager::getAgents() const
{
    return m_agents.values();
}

void DataManager::loadConversations()
{
    // Do nothing
}

void DataManager::addConversation(const std::shared_ptr<Conversation> &conversation)
{
    if (conversation)
    {
        m_conversations.insert(conversation->uuid.trimmed(), conversation);
    }
    else
    {
        LOG_WARN("Attempted to add a null Conversation shared_ptr.");
    }
}

void DataManager::removeConversation(const QString &uuid)
{
    m_conversations.remove(uuid.trimmed());
}

void DataManager::updateConversation(const Conversation &conversation)
{
    auto it = m_conversations.find(conversation.uuid.trimmed());
    if (it != m_conversations.end())
    {
        (*it.value()) = conversation;
        LOG_DEBUG("Updated Conversation with UUID: {}", conversation.uuid);
    }
    else
    {
        LOG_WARN("Conversation with UUID {} not found for update. No action taken.", conversation.uuid);
    }
}

std::shared_ptr<Conversation> DataManager::getConversation(const QString &uuid) const
{
    auto it = m_conversations.find(uuid.trimmed());
    if (it != m_conversations.end())
    {
        return it.value();
    }
    return nullptr;
}

QList<std::shared_ptr<Conversation>> DataManager::getConversations() const
{
    return m_conversations.values();
}