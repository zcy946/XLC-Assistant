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

    connect(this, &DataManager::sig_mcpServersLoaded, this, &DataManager::slot_onMcpServersLoaded);
    connect(DataBaseManager::getInstance()->getWorkerPtr(), &DataBaseWorker::sig_allConversationInfoAcquired, this, &DataManager::slot_handleAllConversationInfoAcquired, Qt::QueuedConnection);
    connect(DataBaseManager::getInstance()->getWorkerPtr(), &DataBaseWorker::sig_messagesAcquired, this, &DataManager::slot_handleMessagesAcquired, Qt::QueuedConnection);
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
    loadConversations(); // 数据库获取对话信息
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

void DataManager::slot_handleAllConversationInfoAcquired(bool success, QJsonArray jsonArrayConversations)
{
    if (!success)
        return;
    for (const QJsonValue &value : jsonArrayConversations)
    {
        if (!value.isObject())
            continue;
        QJsonObject obj = value.toObject();
        QString uuid = obj["uuid"].toString();
        QString agentUuid = obj["agent_uuid"].toString();
        QString summary = obj["summary"].toString();
        QString createdTime = obj["created_time"].toString();
        QString updatedTime = obj["updated_time"].toString();
        int messageCount = obj["message_count"].toInt();

        m_conversations.insert(uuid, Conversation::create(uuid, agentUuid, summary, createdTime, updatedTime, messageCount));
    }
    Q_EMIT sig_conversationsLoaded(success);
}

void DataManager::slot_handleMessagesAcquired(bool success, const QString &conversationUuid, QJsonArray jsonArrayMessages)
{
    if (!success)
        return;

    // 解析消息列表，装载消息
    QList<Message> messages;
    for (const QJsonValue &value : jsonArrayMessages)
    {
        if (!value.isObject())
            continue;
        QJsonObject obj = value.toObject();
        QString temp_uuid = obj["uuid"].toString();
        QString temp_conversationUuid = obj["conversation_uuid"].toString();
        QString temp_roleStr = obj["role"].toString();
        QString temp_content = obj["content"].toString();
        QString temp_createdTime = obj["created_time"].toString();
        QString temp_avatarFilePath = obj["avatar_file_path"].toString();
        QString temp_toolCalls = obj["tool_calls"].toString();
        QString temp_toolCallId = obj["tool_call_id"].toString();

        Message::Role role;
        if (temp_roleStr == "USER")
            role = Message::USER;
        else if (temp_roleStr == "ASSISTANT")
            role = Message::ASSISTANT;
        else if (temp_roleStr == "TOOL")
            role = Message::TOOL;
        else if (temp_roleStr == "SYSTEM")
            role = Message::SYSTEM;
        else
            role = Message::UNKNOWN;

        messages.append(Message(temp_uuid, temp_content, role, temp_createdTime, temp_toolCalls, temp_toolCallId, temp_avatarFilePath));
    }
    std::shared_ptr<Conversation> conversation = getConversation(conversationUuid);
    if (!conversation)
    {
        XLC_LOG_WARN("Handle messages acquired failed (conversationUuid={}): conversation not found", conversationUuid);
        return;
    }
    conversation->loadMessages(messages);
    // 从正在获取消息列表中删除对话
    removePengingConversation(conversationUuid);
    // 通知界面更新消息
    Q_EMIT sig_messagesLoaded(conversationUuid);
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

void DataManager::addLLM(std::shared_ptr<LLM> llm)
{
    if (llm)
    {
        m_llms.insert(llm->uuid.trimmed(), llm);
        saveLLMsAsync();
    }
    else
    {
        XLC_LOG_WARN("Failed to add LLM: Attempted to add a null LLM shared_ptr.");
    }
}

void DataManager::removeLLM(const QString &uuid)
{
    m_llms.remove(uuid.trimmed());
    saveLLMsAsync();
}

void DataManager::updateLLM(std::shared_ptr<LLM> llm)
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
        saveLLMsAsync();
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
            if (filePath.trimmed().isEmpty())
                this->saveLLMs(m_filePathLLMs);
            else
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

void DataManager::addMcpServer(std::shared_ptr<McpServer> mcpServer)
{
    if (mcpServer)
    {
        m_mcpServers.insert(mcpServer->uuid.trimmed(), mcpServer);
        saveMcpServersAsync();
    }
    else
    {
        XLC_LOG_WARN("Attempted to add a null McpServer shared_ptr.");
    }
}

void DataManager::removeMcpServer(const QString &uuid)
{
    m_mcpServers.remove(uuid.trimmed());
    saveMcpServersAsync();
}

void DataManager::updateMcpServer(std::shared_ptr<McpServer> mcpServer)
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
        saveMcpServersAsync();
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
            if (filePath.trimmed().isEmpty())
                this->saveMcpServers(m_filePathMcpServers);
            else
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

void DataManager::addAgent(std::shared_ptr<Agent> agent)
{
    if (agent)
    {
        m_agents.insert(agent->uuid.trimmed(), agent);
        saveAgentsAsync();
    }
    else
    {
        XLC_LOG_WARN("Attempted to add a null Agent shared_ptr.");
    }
}

void DataManager::removeAgent(const QString &uuid)
{
    m_agents.remove(uuid.trimmed());
    saveAgentsAsync();
}

void DataManager::updateAgent(std::shared_ptr<Agent> newAgent)
{
    // TODO 从数据库删除已删除的对话
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
        saveAgentsAsync();

        // 如果系统提示词被修改，更新各个conversation的systemprompt
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
            if (filePath.trimmed().isEmpty())
                this->saveAgents(m_filePathAgents);
            else
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
    // 从数据库加载对话信息，获取数据库conversations表中的各项数据，以及对应的messages的数量，先不获取具体message
    Q_EMIT DataBaseManager::getInstance()->sig_getAllConversationInfo();
    return true;
}

void DataManager::addConversation(std::shared_ptr<Conversation> conversation)
{
    if (conversation)
    {
        m_conversations.insert(conversation->uuid.trimmed(), conversation);
        // 更新Agent配置文件（主要为了更新对话列表）
        saveAgentsAsync();
        // 将对话添加到数据库
        Q_EMIT DataBaseManager::getInstance()->sig_insertNewConversation(conversation->agentUuid,
                                                                         conversation->uuid,
                                                                         conversation->summary,
                                                                         conversation->createdTime,
                                                                         conversation->updatedTime);
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

void DataManager::addPengingConversation(const QString &conversationUuid)
{
    m_pendingConversations.insert(conversationUuid);
}

bool DataManager::isPendingConversations(const QString &conversationUuid)
{
    return m_pendingConversations.contains(conversationUuid);
}

void DataManager::removePengingConversation(const QString &conversationUuid)
{
    m_pendingConversations.remove(conversationUuid);
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
Conversation::Conversation(const QString &agentUuid)
    : uuid(generateUuid()),
      agentUuid(agentUuid),
      createdTime(getCurrentDateTime()),
      updatedTime(getCurrentDateTime()),
      cachedJsonMessages(mcp::json()),
      messageCount(-1)
{
}

Conversation::Conversation(const QString &uuid,
                           const QString &agentUuid,
                           const QString &summary,
                           const QString &createdTime,
                           const QString &updatedTime,
                           int messageCount)
    : uuid(uuid),
      agentUuid(agentUuid),
      summary(summary),
      createdTime(createdTime),
      updatedTime(updatedTime),
      cachedJsonMessages(mcp::json()),
      messageCount(messageCount)
{
}

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
                            const QString &createdTime,
                            const QString &updatedTime,
                            int messageCount)
            : Conversation(uuid, agentUuid, summary, createdTime, updatedTime, messageCount) {}
    };
    return std::static_pointer_cast<Conversation>(std::make_shared<make_shared_enabler>(agentUuid));
    /**
     * NOTE 问题根源：std::make_shared 是外部函数，不能访问 protected 构造函数。
       解决办法：用 make_shared_enabler（内部派生类）作为“桥”，让它调用 protected 构造函数，然后把结果当成 shared_ptr<Conversation> 返回。
       效果：外部只能通过 Conversation::create(...) 来生成对象，既安全又保持 make_shared 的高效内存分配。 */
}

std::shared_ptr<Conversation> Conversation::create(const QString &uuid,
                                                   const QString &agentUuid,
                                                   const QString &summary,
                                                   const QString &createdTime,
                                                   const QString &updatedTime,
                                                   int messageCount)
{
    struct make_shared_enabler : public Conversation
    {
        make_shared_enabler(const QString &uuid,
                            const QString &agentUuid,
                            const QString &summary,
                            const QString &createdTime,
                            const QString &updatedTime,
                            int messageCount)
            : Conversation(uuid, agentUuid, summary, createdTime, updatedTime, messageCount) {}
    };

    return std::static_pointer_cast<Conversation>(std::make_shared<make_shared_enabler>(uuid, agentUuid, summary, createdTime, updatedTime, messageCount));
}

bool Conversation::hasSystemPrompt()
{
    // 只需要保证缓存的json messages有提示词即可，数据库不保存系统提示词
    QMutexLocker locker(&mutex_cachedJsonMessages);
    if (!cachedJsonMessages.is_array() || cachedJsonMessages.empty())
    {
        return false;
    }
    const auto &firstItem = cachedJsonMessages.front();
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
        QMutexLocker locker(&mutex_cachedJsonMessages);
        cachedJsonMessages.front() = systemPromptItem;
    }
    else
    {
        QMutexLocker locker(&mutex_cachedJsonMessages);
        if (cachedJsonMessages.empty())
            cachedJsonMessages.push_back(systemPromptItem);
        else
            cachedJsonMessages.insert(cachedJsonMessages.begin(), systemPromptItem);
    }
}

void Conversation::addMessage(const Message &newMessage)
{
    // 更新 messages
    messages.append(newMessage);
    // 更新本地对话更新时间
    updatedTime = newMessage.createdTime;
    // 更新消息数量
    messageCount += 1;

    // 更新 json messages 缓存
    mcp::json jsonMessage;
    jsonMessage["content"] = newMessage.content.toStdString();
    switch (newMessage.role)
    {
    case Message::USER:
    {
        jsonMessage["role"] = "user";
        QMutexLocker locker(&mutex_cachedJsonMessages);
        cachedJsonMessages.push_back(jsonMessage);
        break;
    }
    case Message::ASSISTANT:
    {
        jsonMessage["role"] = "assistant";
        if (!newMessage.toolCalls.isEmpty())
        {
            try
            {
                mcp::json jsonArrayToolCalls = mcp::json::parse(newMessage.toolCalls.toStdString());
                jsonMessage["tool_calls"] = jsonArrayToolCalls;
            }
            catch (const mcp::json::parse_error &e)
            {
                XLC_LOG_ERROR("Load messages (error={}): JSON Parsing error", e.what());
            }
        }
        QMutexLocker locker(&mutex_cachedJsonMessages);
        cachedJsonMessages.push_back(jsonMessage);
        break;
    }
    case Message::TOOL:
    {
        jsonMessage["role"] = "tool";
        jsonMessage["tool_call_id"] = newMessage.toolCallId.toStdString();
        QMutexLocker locker(&mutex_cachedJsonMessages);
        cachedJsonMessages.push_back(jsonMessage);
        break;
    }
    case Message::SYSTEM:
    {
        jsonMessage["role"] = "system";
        // 清除上下文
        if (newMessage.content == DEFAULT_CONTENT_CLEAR_CONTEXT)
        {
            QMutexLocker locker(&mutex_cachedJsonMessages);
            cachedJsonMessages.clear();
        }
        break;
    }
    default:
    {
        // 其他情况不添加
        break;
    }
    }

    // 插入数据库
    Q_EMIT DataBaseManager::getInstance()->sig_insertNewMessage(uuid,
                                                                newMessage.id,
                                                                static_cast<int>(newMessage.role),
                                                                newMessage.content,
                                                                newMessage.createdTime,
                                                                newMessage.avatarFilePath,
                                                                newMessage.toolCalls,
                                                                newMessage.toolCallId);
}

const QVector<Message> Conversation::getMessages()
{
    /**
     * NOTE 先返回当前messages，
     *      ↓
     *      再将messageCount与当前messages.size()作比较
     *      if (messages.size() != messageCount || messageCount = -1) -> 通知DataBaseManager拉取最新的数据（sig_getMessages(const QString &conversationUuid)），
     *      并将当前conversationUuid加入DataManager的pendingConversations中，表示正在拉取数据，然后发送信号通知界面更新状态。
     *      ↓
     *      在DataManager响应DataBaseManager中获取Messages的信号（sig_resultGetMessages(uuid, QJsonArray[存储Message的json数组])），
     *      在此槽函数中先判断pendingConversations中是否存在对应uuid，如果存在 -> 更新对应conversation的messages（updateMessages(QJsonArray jsonArrayMessages)）
     *      ↓
     *      在updateMessages中，更新messages后触发信号通知页面刷新消息列表
     *  */
    if (messageCount == -1 || messages.size() != messageCount)
    {
        if (!DataManager::getInstance()->isPendingConversations(uuid))
        {
            DataManager::getInstance()->addPengingConversation(uuid);
            Q_EMIT DataBaseManager::getInstance()->sig_getMessageList(uuid);
        }
    }
    return messages;
}

const mcp::json Conversation::getCachedMessages()
{
    QMutexLocker locker(&mutex_cachedJsonMessages);
    return cachedJsonMessages;
}

// 清除上下文
void Conversation::clearContext()
{
    // 加入Role=System的清除上下文消息
    addMessage(Message(DEFAULT_CONTENT_CLEAR_CONTEXT, Message::SYSTEM, getCurrentDateTime()));
}

void Conversation::loadMessages(const QList<Message> &messageList)
{
    QMutexLocker locker(&mutex_cachedJsonMessages);
    cachedJsonMessages.clear();
    messages.clear();
    // 更新消息数量
    messageCount = messageList.size();
    for (const Message &newMessage : messageList)
    {
        // 更新 messages
        messages.append(newMessage);
        // 更新本地对话更新时间
        updatedTime = newMessage.createdTime;

        // 更新 json messages 缓存
        mcp::json jsonMessage;
        jsonMessage["content"] = newMessage.content.toStdString();
        switch (newMessage.role)
        {
        case Message::USER:
        {
            jsonMessage["role"] = "user";
            cachedJsonMessages.push_back(jsonMessage);
            break;
        }
        case Message::ASSISTANT:
        {
            jsonMessage["role"] = "assistant";
            if (!newMessage.toolCalls.isEmpty())
            {
                try
                {
                    mcp::json jsonArrayToolCalls = mcp::json::parse(newMessage.toolCalls.toStdString());
                    jsonMessage["tool_calls"] = jsonArrayToolCalls;
                }
                catch (const mcp::json::parse_error &e)
                {
                    XLC_LOG_ERROR("Load messages (error={}): JSON Parsing error", e.what());
                }
            }
            cachedJsonMessages.push_back(jsonMessage);
            break;
        }
        case Message::TOOL:
        {
            jsonMessage["role"] = "tool";
            jsonMessage["tool_call_id"] = newMessage.toolCallId.toStdString();
            cachedJsonMessages.push_back(jsonMessage);
            break;
        }
        case Message::SYSTEM:
        {
            jsonMessage["role"] = "system";
            // 清除上下文
            if (newMessage.content == DEFAULT_CONTENT_CLEAR_CONTEXT)
                cachedJsonMessages.clear();
            break;
        }
        default:
        {
            // 其他情况不添加
            break;
        }
        }
    }
    XLC_LOG_DEBUG("Load messages successfully (messageCount={})", messageCount);
}