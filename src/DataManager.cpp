#include "DataManager.h"
#include "Logger.hpp"
#include <QFile>
#include <QJsonDocument>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>

DataManager *DataManager::s_instance = nullptr;

DataManager *DataManager::getInstance()
{
    if (!s_instance)
    {
        s_instance = new DataManager();
        // 在应用程序退出时自动清理单例实例
        // TODO 应用关闭时检查是否在保存
        connect(qApp, &QCoreApplication::aboutToQuit, s_instance, &QObject::deleteLater);
    }
    return s_instance;
}

DataManager::DataManager(QObject *parent)
    : QObject(parent)
{
    QFile fileCheck(FILE_MCPSERVERS);
    if (!fileCheck.exists())
    {
        LOG_WARN("McpServers file does not exist: {}", FILE_MCPSERVERS);
    }
    else
    {
        m_filePathMcpServers = FILE_MCPSERVERS;
    }
    fileCheck.setFileName(FILE_AGENTS);
    if (!fileCheck.exists())
    {
        LOG_WARN("Agents file does not exist: {}", FILE_AGENTS);
    }
    else
    {
        m_filePathAgents = FILE_AGENTS;
    }
}

void DataManager::registerAllMetaType()
{
    qRegisterMetaType<McpServer>("McpServer");
    qRegisterMetaType<Agent>("Agent");
    qRegisterMetaType<Conversation>("Conversation");
}

void DataManager::init()
{
    loadDataAsync();
}

void DataManager::loadDataAsync()
{
    loadMcpServersAsync();
    loadAgentsAsync();

    QFuture<bool> futureConversations = QtConcurrent::run(
        [this]()
        {
            return this->loadConversations();
        });
    QFutureWatcher<bool> *futureWatcherConversations = new QFutureWatcher<bool>(this);
    connect(futureWatcherConversations, &QFutureWatcher<bool>::finished, this,
            [this, futureWatcherConversations]()
            {
                bool success = futureWatcherConversations->result();
                Q_EMIT sig_conversationsLoaded(success);
                if (success)
                {
                    LOG_INFO("Successfully loaded [{}] Conversations from database", m_conversations.count());
                }
                futureWatcherConversations->deleteLater();
            });
    futureWatcherConversations->setFuture(futureConversations);
}

void DataManager::loadMcpServersAsync()
{
    QFuture<bool> futureMcpServers = QtConcurrent::run(
        [this]()
        {
            return this->loadMcpServers(m_filePathMcpServers);
        });
    QFutureWatcher<bool> *futureWatcherMcpServers = new QFutureWatcher<bool>(this);
    connect(futureWatcherMcpServers, &QFutureWatcher<bool>::finished, this,
            [this, futureWatcherMcpServers]()
            {
                bool success = futureWatcherMcpServers->result();
                Q_EMIT sig_mcpServersLoaded(success);
                if (success)
                {
                    LOG_INFO("Successfully loaded [{}] McpServers from: [{}]", m_mcpServers.count(), m_filePathMcpServers);
                }
                futureWatcherMcpServers->deleteLater();
            });
    futureWatcherMcpServers->setFuture(futureMcpServers);
}

void DataManager::loadAgentsAsync()
{
    QFuture<bool> futureAgents = QtConcurrent::run(
        [this]()
        {
            return this->loadAgents(m_filePathAgents);
        });
    QFutureWatcher<bool> *futureWatcherAgents = new QFutureWatcher<bool>(this);
    connect(futureWatcherAgents, &QFutureWatcher<bool>::finished, this,
            [this, futureWatcherAgents]()
            {
                bool success = futureWatcherAgents->result();
                Q_EMIT sig_agentsLoaded(success);
                if (success)
                {
                    LOG_INFO("Successfully loaded [{}] Agents from: [{}]", m_agents.count(), m_filePathMcpServers);
                }
                futureWatcherAgents->deleteLater();
            });
    futureWatcherAgents->setFuture(futureAgents);
}

bool DataManager::loadMcpServers(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString errorMsg = QString("Could not open McpServers file: %1 - %2").arg(filePath).arg(file.errorString());
        LOG_ERROR("{}", errorMsg);
        return false;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull())
    {
        QString errorMsg = QString("Failed to create JSON document from file: %1").arg(filePath);
        LOG_ERROR("{}", errorMsg);
        return false;
    }

    if (!doc.isArray())
    {
        QString errorMsg = QString("McpServers JSON root is not an array in file: %1").arg(filePath);
        LOG_ERROR("{}", errorMsg);
        return false;
    }

    QJsonArray jsonArray = doc.array();

    // 清空现有数据，准备加载新数据
    m_mcpServers.clear();

    for (const QJsonValue &value : jsonArray)
    {
        if (value.isObject())
        {
            QJsonObject serverObject = value.toObject();
            // 使用 McpServer 的 fromJson 辅助函数来解析
            McpServer newServer = McpServer::fromJson(serverObject);

            // 检查 UUID 是否有效
            if (newServer.uuid.isEmpty())
            {
                LOG_WARN("McpServer entry missing UUID. Skipping entry.");
                continue;
            }

            // 使用 std::make_shared 创建智能指针并存储到 QHash 中
            m_mcpServers.insert(newServer.uuid, std::make_shared<McpServer>(newServer));
            LOG_DEBUG("Loaded McpServer: {}, UUID: {}", newServer.name, newServer.uuid);
        }
        else
        {
            LOG_WARN("McpServers array contains non-object element. Skipping.");
        }
    }
    return true;
}

void DataManager::addMcpServer(const std::shared_ptr<McpServer> &mcpServer)
{
    if (mcpServer)
    {
        m_mcpServers.insert(mcpServer->uuid.trimmed(), mcpServer);
        saveMcpServersAsync(m_filePathMcpServers);
    }
    else
    {
        LOG_WARN("Attempted to add a null McpServer shared_ptr.");
    }
}

void DataManager::removeMcpServer(const QString &uuid)
{
    m_mcpServers.remove(uuid.trimmed());
    saveMcpServersAsync(m_filePathMcpServers);
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
        saveMcpServersAsync(FILE_MCPSERVERS);
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

void DataManager::saveMcpServers(const QString &filePath) const
{
    QJsonArray jsonArray;
    for (const auto &serverPtr : m_mcpServers.values())
    {
        if (serverPtr)
        {
            jsonArray.append(serverPtr->toJsonObject());
        }
    }

    QJsonDocument doc(jsonArray);

    QFile file(filePath);
    // 使用 QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate
    // WriteOnly: 只写模式
    // Text: 文本模式（处理行末符）
    // Truncate: 如果文件存在，先清空其内容
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        QString errorMsg = QString("Could not open McpServers file for writing: %1 - %2").arg(filePath).arg(file.errorString());
        LOG_ERROR("{}", errorMsg);
        return;
    }

    // 将 JSON 文档写入文件，使用 BeautifulIndented 格式使其可读性更好
    QByteArray jsonData = doc.toJson(QJsonDocument::JsonFormat::Indented);
    qint64 bytesWritten = file.write(jsonData);
    file.close();

    if (bytesWritten == -1)
    {
        QString errorMsg = QString("Failed to write to McpServers file: %1 - %2").arg(filePath).arg(file.errorString());
        LOG_ERROR("{}", errorMsg);
        return;
    }

    LOG_INFO("Successfully saved [{}] McpServers to: [{}]", m_mcpServers.count(), filePath);
}

void DataManager::saveMcpServersAsync(const QString &filePath) const
{
    QFuture<void> futureMcpServers = QtConcurrent::run(
        [this, filePath]()
        {
            this->saveMcpServers(filePath);
        });
    QFutureWatcher<void> *futureWatcherMcpServers = new QFutureWatcher<void>();
    connect(futureWatcherMcpServers, &QFutureWatcher<void>::finished, this,
            [this, futureWatcherMcpServers, filePath]()
            {
                LOG_DEBUG("Asynchronous save of McpServers finished for: [{}]", filePath);
                futureWatcherMcpServers->deleteLater();
            });
    futureWatcherMcpServers->setFuture(futureMcpServers);
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

bool DataManager::loadAgents(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString errorMsg = QString("Could not open Agents file: %1 - %2").arg(filePath).arg(file.errorString());
        LOG_ERROR("{}", errorMsg);
        return false;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull())
    {
        QString errorMsg = QString("Failed to create JSON document from file: %1").arg(filePath);
        LOG_ERROR("{}", errorMsg);
        return false;
    }

    if (!doc.isArray())
    {
        QString errorMsg = QString("Agents JSON root is not an array in file: %1").arg(filePath);
        LOG_ERROR("{}", errorMsg);
        return false;
    }

    QJsonArray jsonArray = doc.array();

    // 清空现有数据，准备加载新数据
    m_agents.clear();

    for (const QJsonValue &value : jsonArray)
    {
        if (value.isObject())
        {
            QJsonObject agentObject = value.toObject();
            // 使用 Agent 的 fromJson 辅助函数来解析
            Agent newAgent = Agent::fromJson(agentObject);

            // 检查 UUID 是否有效
            if (newAgent.uuid.isEmpty())
            {
                LOG_WARN("Agent entry missing UUID. Skipping entry.");
                continue;
            }

            // 使用 std::make_shared 创建智能指针并存储到 QHash 中
            m_agents.insert(newAgent.uuid, std::make_shared<Agent>(newAgent));
            LOG_DEBUG("Loaded Agent: {}, UUID: {}", newAgent.name, newAgent.uuid);
        }
        else
        {
            LOG_WARN("Agents array contains non-object element. Skipping.");
        }
    }
    return true;
}

void DataManager::addAgent(const std::shared_ptr<Agent> &agent)
{
    if (agent)
    {
        m_agents.insert(agent->uuid.trimmed(), agent);
        saveAgentsAsync(m_filePathAgents);
    }
    else
    {
        LOG_WARN("Attempted to add a null Agent shared_ptr.");
    }
}

void DataManager::removeAgent(const QString &uuid)
{
    m_agents.remove(uuid.trimmed());
    saveAgentsAsync(m_filePathAgents);
}

void DataManager::updateAgent(const Agent &agent)
{
    auto it = m_agents.find(agent.uuid.trimmed());
    if (it != m_agents.end())
    {
        (*it.value()) = agent;
        LOG_DEBUG("Updated Agent with UUID: {}", agent.uuid);
        saveAgentsAsync(m_filePathAgents);
    }
    else
    {
        LOG_WARN("Agent with UUID {} not found for update. No action taken.", agent.uuid);
    }
}

void DataManager::DataManager::saveAgents(const QString &filePath) const
{
    QJsonArray jsonArray;
    for (const auto &agentPtr : m_agents.values())
    {
        if (agentPtr)
        {
            jsonArray.append(agentPtr->toJsonObject());
        }
    }

    QJsonDocument doc(jsonArray);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        QString errorMsg = QString("Could not open Agents file for writing: %1 - %2").arg(filePath).arg(file.errorString());
        LOG_ERROR("{}", errorMsg);
        return;
    }

    QByteArray jsonData = doc.toJson(QJsonDocument::JsonFormat::Indented);
    qint64 bytesWritten = file.write(jsonData);
    file.close();

    if (bytesWritten == -1)
    {
        QString errorMsg = QString("Failed to write to Agents file: %1 - %2").arg(filePath).arg(file.errorString());
        LOG_ERROR("{}", errorMsg);
        return;
    }

    LOG_INFO("Successfully saved [{}] Agents to: [{}]", m_agents.count(), filePath);
}

void DataManager::saveAgentsAsync(const QString &filePath) const
{
    QFuture<void> futureAgents = QtConcurrent::run(
        [this, filePath]()
        {
            this->saveAgents(filePath);
        });
    QFutureWatcher<void> *futureWatcherAgents = new QFutureWatcher<void>();
    connect(futureWatcherAgents, &QFutureWatcher<void>::finished, this,
            [this, futureWatcherAgents, filePath]()
            {
                LOG_DEBUG("Asynchronous save of Agents finished for: [{}]", filePath);
                futureWatcherAgents->deleteLater();
            });
    futureWatcherAgents->setFuture(futureAgents);
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

bool DataManager::loadConversations()
{
    // TODO 从数据库加载对话信息
    return true;
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

void DataManager::setFilePathMcpServers(const QString &filePath)
{
    if (filePath.isEmpty())
        return;
    m_filePathMcpServers = filePath;
    loadMcpServersAsync();
    Q_EMIT sig_filePathChangedMcpServers(filePath);
}

const QString &DataManager::getFilePathMcpServers() const
{
    return m_filePathMcpServers;
}

void DataManager::setFilePathAgents(const QString &filePath)
{
    if (filePath.isEmpty())
        return;
    m_filePathAgents = filePath;
    loadAgentsAsync();
    Q_EMIT sig_filePathChangedAgents(filePath);
}

const QString &DataManager::getFilePathAgents() const
{
    return m_filePathAgents;
}