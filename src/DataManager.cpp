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
    // 检测配置文件
    if (!QFile(FILE_LLMS).exists())
        XLC_LOG_WARN("LLMs file does not exist (file={})", FILE_LLMS);
    else
        m_filePathLLMs = FILE_LLMS;
    if (!QFile(FILE_MCPSERVERS).exists())
        XLC_LOG_WARN("McpServers file does not exist (file={})", FILE_MCPSERVERS);
    else
        m_filePathMcpServers = FILE_MCPSERVERS;
    if (!QFile(FILE_AGENTS).exists())
        XLC_LOG_WARN("Agents file does not exist (file={})", FILE_AGENTS);
    else
        m_filePathAgents = FILE_AGENTS;

    // 处理MCPServers加载完毕信号
    connect(this, &DataManager::sig_mcpServersLoaded, this, &DataManager::slot_onMcpServersLoaded);
}

// void DataManager::registerAllMetaType()
// {
//     qRegisterMetaType<McpServer>("McpServer");
//     qRegisterMetaType<Agent>("Agent");
//     qRegisterMetaType<Conversation>("Conversation");
// }

void DataManager::init()
{
    loadDataAsync();
}

void DataManager::loadDataAsync()
{
    loadLLMsAsync();
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
                    XLC_LOG_INFO("Loaded conversations from database (count={})", m_conversations.count());
                }
                futureWatcherConversations->deleteLater();
            });
    futureWatcherConversations->setFuture(futureConversations);
}

void DataManager::loadLLMsAsync()
{
    QFuture<bool> futureMcpServers = QtConcurrent::run(
        [this]()
        {
            return this->loadLLMs(m_filePathLLMs);
        });
    QFutureWatcher<bool> *futureWatcherLLMs = new QFutureWatcher<bool>(this);
    connect(futureWatcherLLMs, &QFutureWatcher<bool>::finished, this,
            [this, futureWatcherLLMs]()
            {
                bool success = futureWatcherLLMs->result();
                Q_EMIT sig_LLMsLoaded(success);
                if (success)
                {
                    XLC_LOG_INFO("Successfully loaded LLMs (count={}, path={})", m_llms.count(), QFileInfo(m_filePathLLMs).absoluteFilePath());
                }
                futureWatcherLLMs->deleteLater();
            });
    futureWatcherLLMs->setFuture(futureMcpServers);
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
                    XLC_LOG_INFO("Successfully loaded MCPServers (count={}, filePath={})", m_mcpServers.count(), QFileInfo(m_filePathMcpServers).absoluteFilePath());
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
                    XLC_LOG_INFO("Successfully loaded agents (count={}, filePath={})", m_agents.count(), QFileInfo(m_filePathAgents).absoluteFilePath());
                }
                futureWatcherAgents->deleteLater();
            });
    futureWatcherAgents->setFuture(futureAgents);
}

void DataManager::slot_onMcpServersLoaded(bool success)
{
    if (!success)
        return;
}

bool DataManager::loadLLMs(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString errorMsg = QString("Could not open LLMs file: %1 - %2").arg(filePath).arg(file.errorString());
        XLC_LOG_ERROR("{}", errorMsg);
        return false;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull())
    {
        QString errorMsg = QString("Failed to create JSON document from file: %1").arg(filePath);
        XLC_LOG_ERROR("{}", errorMsg);
        return false;
    }

    if (!doc.isArray())
    {
        QString errorMsg = QString("LLMs JSON root is not an array in file: %1").arg(filePath);
        XLC_LOG_ERROR("{}", errorMsg);
        return false;
    }

    QJsonArray jsonArray = doc.array();

    // 清空现有数据，准备加载新数据
    m_llms.clear();

    for (const QJsonValue &value : jsonArray)
    {
        if (value.isObject())
        {
            QJsonObject llmObject = value.toObject();
            // 使用 LLM 的 fromJson 辅助函数来解析
            LLM newLLM = LLM::fromJson(llmObject);

            // 检查 UUID 是否有效
            if (newLLM.uuid.isEmpty())
            {
                XLC_LOG_WARN("Failed to load LLMs: LLM entry missing UUID. Skipping.");
                continue;
            }

            // 使用 std::make_shared 创建智能指针并存储到 QHash 中
            m_llms.insert(newLLM.uuid, std::make_shared<LLM>(newLLM));
            XLC_LOG_TRACE("Successed to load LLM (modelId={}, modelName={})", newLLM.modelID, newLLM.modelName);
        }
        else
        {
            XLC_LOG_WARN("Failed to load LLMs: LLMs array contains non-object element. Skipping.");
        }
    }
    return true;
}

void DataManager::addLLM(const std::shared_ptr<LLM> &llm)
{
    if (llm)
    {
        m_llms.insert(llm->uuid.trimmed(), llm);
        saveLLMsAsync(m_filePathLLMs);
    }
    else
    {
        XLC_LOG_WARN("Failed to add LLM: Attempted to add a null LLM shared_ptr.");
    }
}

void DataManager::removeLLM(const QString &uuid)
{
    m_llms.remove(uuid.trimmed());
    saveLLMsAsync(m_filePathLLMs);
}

void DataManager::updateLLM(const std::shared_ptr<LLM> &llm)
{
    if (!llm)
    {
        XLC_LOG_WARN("Failed to update LLM: Attempted to update a null LLM shared_ptr.");
        return;
    }
    auto it = m_llms.find(llm->uuid.trimmed());
    if (it != m_llms.end())
    {
        (*it.value()) = *llm;
        XLC_LOG_DEBUG("Successed to update LLM (uuid={})", llm->uuid);
        saveLLMsAsync(m_filePathLLMs);
    }
    else
    {
        XLC_LOG_WARN("Failed to update LLM (LLMUuid={}): LLM not found", llm->uuid);
    }
}

void DataManager::saveLLMs(const QString &filePath) const
{
    QJsonArray jsonArray;
    for (const auto &llmPtr : m_llms.values())
    {
        if (llmPtr)
        {
            jsonArray.append(llmPtr->toJsonObject());
        }
    }

    QJsonDocument doc(jsonArray);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        QString errorMsg = QString("Failed to save LLMs (filepath=%1, error=%2): Could not open LLMs file").arg(filePath).arg(file.errorString());
        XLC_LOG_ERROR("{}", errorMsg);
        return;
    }

    QByteArray jsonData = doc.toJson(QJsonDocument::JsonFormat::Indented);
    qint64 bytesWritten = file.write(jsonData);
    file.close();

    if (bytesWritten == -1)
    {
        QString errorMsg = QString("Failed to save LLMs (filepath=%1, error=%2): Failed to write to LLMs file").arg(filePath).arg(file.errorString());
        XLC_LOG_ERROR("{}", errorMsg);
        return;
    }

    XLC_LOG_INFO("Save LLMS successed (count={}, filepath={})", m_llms.count(), QFileInfo(filePath).absoluteFilePath());
}

void DataManager::saveLLMsAsync(const QString &filePath) const
{
    QFuture<void> futureLLMs = QtConcurrent::run(
        [this, filePath]()
        {
            this->saveLLMs(filePath);
        });
    QFutureWatcher<void> *futureWatcherLLMs = new QFutureWatcher<void>();
    connect(futureWatcherLLMs, &QFutureWatcher<void>::finished, this,
            [this, futureWatcherLLMs, filePath]()
            {
                XLC_LOG_DEBUG("Asynchronous save of LLMs finished (filepath={})", QFileInfo(filePath).absoluteFilePath());
                futureWatcherLLMs->deleteLater();
            });
    futureWatcherLLMs->setFuture(futureLLMs);
}

std::shared_ptr<LLM> DataManager::getLLM(const QString &uuid) const
{
    if (uuid.isEmpty())
        return nullptr;
    auto it = m_llms.find(uuid.trimmed());
    if (it != m_llms.end())
    {
        return it.value();
    }
    return nullptr;
}

QList<std::shared_ptr<LLM>> DataManager::getLLMs() const
{
    return m_llms.values();
}

void DataManager::setFilePathLLMs(const QString &filePath)
{

    if (filePath.isEmpty())
        return;
    m_filePathLLMs = filePath;
    loadLLMsAsync();
    Q_EMIT sig_LLMsFilePathChange(filePath);
}

const QString &DataManager::getFilePathLLMs() const
{
    return m_filePathLLMs;
}

bool DataManager::loadMcpServers(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString errorMsg = QString("Load MCP Servers failed(filepath=%1, errormessage=%2): Could not open McpServers file").arg(filePath).arg(file.errorString());
        XLC_LOG_ERROR("{}", errorMsg);
        return false;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull())
    {
        QString errorMsg = QString("Load MCP Servers failed (filepath=%1): Failed to create JSONDocument from file").arg(filePath);
        XLC_LOG_ERROR("{}", errorMsg);
        return false;
    }

    if (!doc.isArray())
    {
        QString errorMsg = QString("Load MCP Servers failed (filepath=%1): McpServers JSON root is not an array in file").arg(filePath);
        XLC_LOG_ERROR("{}", errorMsg);
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
                XLC_LOG_WARN("McpServer entry missing UUID. Skipping entry.");
                continue;
            }

            // 使用 std::make_shared 创建智能指针并存储到 QHash 中
            m_mcpServers.insert(newServer.uuid, std::make_shared<McpServer>(newServer));
            XLC_LOG_TRACE("Loaded server (name={}, uuid={})", newServer.name, newServer.uuid);
        }
        else
        {
            XLC_LOG_WARN("McpServers array contains non-object element. Skipping.");
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
        XLC_LOG_WARN("Attempted to add a null McpServer shared_ptr.");
    }
}

void DataManager::removeMcpServer(const QString &uuid)
{
    m_mcpServers.remove(uuid.trimmed());
    saveMcpServersAsync(m_filePathMcpServers);
}

void DataManager::updateMcpServer(const std::shared_ptr<McpServer> &mcpServer)
{
    if (!mcpServer)
    {
        XLC_LOG_WARN("Attempted to update a null McpServer shared_ptr.");
        return;
    }
    auto it = m_mcpServers.find(mcpServer->uuid.trimmed());
    if (it != m_mcpServers.end())
    {
        (*it.value()) = *mcpServer;
        XLC_LOG_DEBUG("Updated McpServer with UUID: {}", mcpServer->uuid);
        saveMcpServersAsync(m_filePathMcpServers);
    }
    else
    {
        XLC_LOG_WARN("McpServer with UUID {} not found for update. No action taken.", mcpServer->uuid);
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
        XLC_LOG_ERROR("{}", errorMsg);
        return;
    }

    // 将 JSON 文档写入文件，使用 BeautifulIndented 格式使其可读性更好
    QByteArray jsonData = doc.toJson(QJsonDocument::JsonFormat::Indented);
    qint64 bytesWritten = file.write(jsonData);
    file.close();

    if (bytesWritten == -1)
    {
        QString errorMsg = QString("Failed to write to McpServers file: %1 - %2").arg(filePath).arg(file.errorString());
        XLC_LOG_ERROR("{}", errorMsg);
        return;
    }

    XLC_LOG_INFO("Successfully saved [{}] McpServers to: [{}]", m_mcpServers.count(), QFileInfo(filePath).absoluteFilePath());
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
                XLC_LOG_DEBUG("Asynchronous save of McpServers finished for: [{}]", QFileInfo(filePath).absoluteFilePath());
                futureWatcherMcpServers->deleteLater();
            });
    futureWatcherMcpServers->setFuture(futureMcpServers);
}

std::shared_ptr<McpServer> DataManager::getMcpServer(const QString &uuid) const
{
    if (uuid.isEmpty())
        return nullptr;
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

void DataManager::setFilePathMcpServers(const QString &filePath)
{
    if (filePath.isEmpty())
        return;
    m_filePathMcpServers = filePath;
    loadMcpServersAsync();
    Q_EMIT sig_mcpServersFilePathChange(filePath);
}

const QString &DataManager::getFilePathMcpServers() const
{
    return m_filePathMcpServers;
}

bool DataManager::loadAgents(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString errorMsg = QString("Could not open Agents file: %1 - %2").arg(filePath).arg(file.errorString());
        XLC_LOG_ERROR("{}", errorMsg);
        return false;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull())
    {
        QString errorMsg = QString("Failed to create JSON document from file: %1").arg(filePath);
        XLC_LOG_ERROR("{}", errorMsg);
        return false;
    }

    if (!doc.isArray())
    {
        QString errorMsg = QString("Agents JSON root is not an array in file: %1").arg(filePath);
        XLC_LOG_ERROR("{}", errorMsg);
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
                XLC_LOG_WARN("Agent entry missing UUID. Skipping entry.");
                continue;
            }

            // 使用 std::make_shared 创建智能指针并存储到 QHash 中
            m_agents.insert(newAgent.uuid, std::make_shared<Agent>(newAgent));
            XLC_LOG_TRACE("Loaded Agent: {}, UUID: {}", newAgent.name, newAgent.uuid);
        }
        else
        {
            XLC_LOG_WARN("Agents array contains non-object element. Skipping.");
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
        XLC_LOG_WARN("Attempted to add a null Agent shared_ptr.");
    }
}

void DataManager::removeAgent(const QString &uuid)
{
    m_agents.remove(uuid.trimmed());
    saveAgentsAsync(m_filePathAgents);
}

void DataManager::updateAgent(const std::shared_ptr<Agent> &newAgent)
{
    if (!newAgent)
    {
        XLC_LOG_WARN("Update agent failed (uuid={}): attempted to update a null Agent shared_ptr", newAgent->uuid);
        return;
    }
    auto it = m_agents.find(newAgent->uuid.trimmed());
    if (it != m_agents.end())
    {
        bool isSystemPromptModified = false;
        if (newAgent->systemPrompt != (*it)->systemPrompt)
            isSystemPromptModified = true;

        (*it.value()) = *newAgent;
        Q_EMIT sig_agentUpdate(newAgent->uuid);
        XLC_LOG_DEBUG("Updated Agent successed (uuid={})", newAgent->uuid);
        saveAgentsAsync(m_filePathAgents);

        // 更新各个conversation的systemprompt
        if (isSystemPromptModified)
        {
            for (const QString &conversationUuid : (*it)->conversations)
            {
                std::shared_ptr<Conversation> conversation = DataManager::getInstance()->getConversation(conversationUuid);
                if (conversation)
                    conversation->resetSystemPrompt();
            }
        }
    }
    else
    {
        XLC_LOG_WARN("Agent with UUID {} not found for update. No action taken.", newAgent->uuid);
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
        XLC_LOG_ERROR("{}", errorMsg);
        return;
    }

    QByteArray jsonData = doc.toJson(QJsonDocument::JsonFormat::Indented);
    qint64 bytesWritten = file.write(jsonData);
    file.close();

    if (bytesWritten == -1)
    {
        QString errorMsg = QString("Failed to write to Agents file: %1 - %2").arg(filePath).arg(file.errorString());
        XLC_LOG_ERROR("{}", errorMsg);
        return;
    }
    XLC_LOG_INFO("Successfully saved [{}] Agents to: [{}]", m_agents.count(), QFileInfo(filePath).absoluteFilePath());
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
                XLC_LOG_DEBUG("Asynchronous save of Agents finished for: [{}]", QFileInfo(filePath).absoluteFilePath());
                futureWatcherAgents->deleteLater();
            });
    futureWatcherAgents->setFuture(futureAgents);
}

std::shared_ptr<Agent> DataManager::getAgent(const QString &uuid) const
{
    if (uuid.isEmpty())
        return nullptr;
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

void DataManager::setFilePathAgents(const QString &filePath)
{
    if (filePath.isEmpty())
        return;
    m_filePathAgents = filePath;
    loadAgentsAsync();
    Q_EMIT sig_agentsFilePathChange(filePath);
}

const QString &DataManager::getFilePathAgents() const
{
    return m_filePathAgents;
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
        XLC_LOG_WARN("Attempted to add a null Conversation shared_ptr.");
    }
}

void DataManager::removeConversation(const QString &uuid)
{
    m_conversations.remove(uuid.trimmed());
}

void DataManager::updateConversation(std::shared_ptr<Conversation> newConversation)
{
    QString uuid = newConversation->uuid.trimmed();
    auto it = m_conversations.find(uuid);
    if (it != m_conversations.end())
    {
        it.value() = newConversation;
        XLC_LOG_DEBUG("Updated Conversation with UUID: {}", uuid);
    }
    else
    {
        XLC_LOG_WARN("Conversation with UUID {} not found for update. No action taken.", uuid);
    }
}

std::shared_ptr<Conversation> DataManager::getConversation(const QString &uuid) const
{
    if (uuid.isEmpty())
        return nullptr;
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

std::shared_ptr<Conversation> DataManager::createNewConversation(const QString &agentUuid)
{
    std::shared_ptr<Agent> targetAgent = getAgent(agentUuid);
    if (!targetAgent)
    {
        XLC_LOG_ERROR("Failed to create new conversation (agentUuid={}): agent not found ", agentUuid);
        return nullptr;
    }
    std::shared_ptr<Conversation> newConversation = Conversation::create(agentUuid);
    // 更新对应agent的对话列表
    targetAgent->conversations.insert(newConversation->uuid);
    return newConversation;
}