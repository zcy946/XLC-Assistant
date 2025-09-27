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

void DataManager::registerAllMetaType()
{
    // qRegisterMetaType<McpServer>("McpServer");
    // qRegisterMetaType<Agent>("Agent");
    // qRegisterMetaType<Conversation>("Conversation");
    // qRegisterMetaType<Message>("Message");
}

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

// LLM
LLM::LLM()
    : uuid(generateUuid()),
      modelID(),
      modelName(),
      apiKey(),
      baseUrl(),
      endpoint("/v1/chat/completions")
{
}

LLM::LLM(const QString &modelID,
         const QString &modelName,
         const QString &apiKey,
         const QString &baseUrl,
         const QString &endpoint)
    : uuid(generateUuid()),
      modelID(modelID),
      modelName(modelName),
      apiKey(apiKey),
      baseUrl(baseUrl),
      endpoint(endpoint)
{
}

LLM LLM::fromJson(const QJsonObject &jsonObject)
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

QJsonObject LLM::toJsonObject() const
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

// McpServer
McpServer::McpServer()
    : isActive(false),
      uuid(generateUuid()),
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

McpServer::McpServer(bool isActive,
                     const QString &name,
                     const QString &description,
                     Type type,
                     int timeout,
                     const QString &command,
                     const QVector<QString> &args,
                     const QMap<QString, QString> &envVars,
                     const QString &baseUrl,
                     const QString &endpoint,
                     const QString &requestHeaders)
    : isActive(isActive),
      uuid(generateUuid()),
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

McpServer::McpServer(bool isActive,
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
                     const QString &requestHeaders)
    : isActive(isActive),
      uuid(generateUuid()),
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

McpServer McpServer::fromJson(const QJsonObject &jsonObject)
{
    McpServer server;
    server.isActive = jsonObject["isActive"].toBool();
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
QJsonObject McpServer::toJsonObject() const
{
    QJsonObject jsonObject;
    jsonObject["isActive"] = isActive;
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

// Agent
Agent::Agent()
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

Agent::Agent(const QString &name,
             const QString &description,
             int context,
             const QString &systemPrompt,
             const QString &llmUUid,
             double temperature,
             double topP,
             int maxTokens,
             const QSet<QString> &mcpServers,
             const QSet<QString> &conversations)
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
Agent Agent::fromJson(const QJsonObject &jsonObject)
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

QJsonObject Agent::toJsonObject() const
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

// Conversation
std::shared_ptr<Conversation> Conversation::create(const QString &agentUuid)
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

std::shared_ptr<Conversation> Conversation::create(const QString &uuid,
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

bool Conversation::hasSystemPrompt()
{
    QMutexLocker locker(&mutex);
    if (!messages.is_array() || messages.empty())
    {
        return false;
    }
    const auto &firstItem = messages.front();
    if (!firstItem.is_object())
    {
        return false;
    }
    if (firstItem.contains("role") && firstItem["role"].is_string() && firstItem["role"] == "system")
    {
        return true;
    }
    return false;
}

void Conversation::resetSystemPrompt()
{
    std::shared_ptr<Agent> agent = DataManager::getInstance()->getAgent(agentUuid);
    if (!agent)
    {
        return;
    }
    if (agent->systemPrompt.isEmpty())
        return;

    mcp::json systemPromptItem = mcp::json();
    systemPromptItem["content"] = agent->systemPrompt.toStdString();
    systemPromptItem["role"] = "system";

    if (hasSystemPrompt())
    {
        QMutexLocker locker(&mutex);
        messages.front() = systemPromptItem;
    }
    else
    {
        QMutexLocker locker(&mutex);
        if (messages.empty())
            messages.push_back(systemPromptItem);
        else
            messages.insert(messages.begin(), systemPromptItem);
    }
}

void Conversation::addMessage(const mcp::json &newMessage)
{
    QMutexLocker locker(&mutex);
    messages.push_back(newMessage);
    updatedTime = QDateTime::currentDateTime();

    // 插入数据库
    QString message = QString::fromStdString(newMessage["content"].get<std::string>());
    Message::Role role;
    std::string strRole = newMessage["role"].get<std::string>();
    if (strRole == "user")
        role = Message::USER;
    else if (strRole == "assistant")
        role = Message::ASSISTANT;
    else if (strRole == "tool")
        role = Message::SYSTEM;
    else
        role = Message::SYSTEM;
    Message temp_message(message, role);
    Q_EMIT DataBaseManager::getInstance()->sig_insertNewMessage(uuid, temp_message.id,
                                                                static_cast<int>(temp_message.role),
                                                                temp_message.text,
                                                                temp_message.createdTime,
                                                                temp_message.avatarFilePath);
}

const mcp::json Conversation::getMessages()
{
    QMutexLocker locker(&mutex);
    return messages;
}

// 清除上下文
void Conversation::clearContext()
{
    QMutexLocker locker(&mutex);
    messages.clear();
}

Conversation::Conversation(const QString &agentUuid)
    : uuid(generateUuid()),
      agentUuid(agentUuid),
      createdTime(QDateTime::currentDateTime()),
      updatedTime(QDateTime::currentDateTime()),
      messages(mcp::json())
{
}

Conversation::Conversation(const QString &uuid,
             const QString &agentUuid,
             const QString &summary,
             const QDateTime &createdTime,
             const QDateTime &updatedTime)
    : uuid(uuid),
      agentUuid(agentUuid),
      summary(summary),
      createdTime(createdTime),
      updatedTime(updatedTime),
      messages(mcp::json())
{
}