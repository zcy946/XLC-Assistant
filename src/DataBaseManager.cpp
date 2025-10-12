#include "DataBaseManager.h"
#include <QCoreApplication>
#include "Logger.hpp"
#include <QSqlError>
#include <QSqlQuery>
#include <QtConcurrent>
#include "ToastManager.h"

DataBaseManager::DataBaseManager(QObject *parent)
    : QObject(parent)
{
    // 创建数据库线程
    m_worker = new DataBaseWorker(DATABASE_FILENAME);
    m_worker->moveToThread(&m_thread);

    connect(qApp, &QCoreApplication::aboutToQuit, &m_thread, &QThread::quit);

    connect(&m_thread, &QThread::started, m_worker, &DataBaseWorker::slot_initialize); // 在线程启动后初始化数据库，确保 QSqlDatabase 和 worker 在同一线程
    connect(this, &DataBaseManager::sig_getAllConversationInfo, m_worker, &DataBaseWorker::slot_getAllConversationInfo, Qt::QueuedConnection);
    connect(this, &DataBaseManager::sig_insertNewConversation, m_worker, &DataBaseWorker::slot_insertNewConversation, Qt::QueuedConnection);
    connect(this, &DataBaseManager::sig_insertNewMessage, m_worker, &DataBaseWorker::slot_insertNewMessage, Qt::QueuedConnection);
    connect(this, &DataBaseManager::sig_getMessageList, m_worker, &DataBaseWorker::slot_getMessages, Qt::QueuedConnection);
    connect(this, &DataBaseManager::sig_deleteConversation, m_worker, &DataBaseWorker::slot_deleteConversation, Qt::QueuedConnection);

    m_thread.start();
}

DataBaseManager::~DataBaseManager()
{
    m_thread.quit();
    m_thread.wait();

    // 释放 worker
    if (m_worker)
    {
        m_worker->deleteLater();
        m_worker = nullptr;
    }
}

const DataBaseWorker *DataBaseManager::getWorkerPtr()
{
    return m_worker;
}

// DataBaseWorker
DataBaseWorker::DataBaseWorker(const QString &dataBaseFile, QObject *parent)
    : QObject(parent), m_dataBaseFile(dataBaseFile)
{
}

DataBaseWorker::~DataBaseWorker()
{
    if (m_dataBase.isOpen())
        m_dataBase.close();

    if (QSqlDatabase::contains(DB_CONNECTION_NAME))
    {
        QSqlDatabase::removeDatabase(DB_CONNECTION_NAME);
    }
}

void DataBaseWorker::initializeDatabase()
{
    // 创建 conversations 表
    QSqlQuery query(m_dataBase);
    if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS conversations (
                id TEXT PRIMARY KEY,
                agent_id TEXT NOT NULL,
                summary TEXT, 
                created_time TEXT NOT NULL,
                updated_time TEXT NOT NULL
            );
        )"))
    {
        XLC_LOG_WARN("Initialize database failed (query={}): {}", query.lastQuery(), query.lastError().text());
        ToastManager::showMessage(Toast::Type::Warning, QString("初始化数据库链接失败: %1").arg(query.lastError().text()));
        return;
    }
    // 创建 idx_conversations_agent_id 索引
    if (!query.exec(R"(
            CREATE INDEX IF NOT EXISTS idx_conversations_agent_id ON conversations(agent_id);
        )"))
    {
        XLC_LOG_WARN("Initialize database (query={}): {}", query.lastQuery(), query.lastError().text());
        ToastManager::showMessage(Toast::Type::Warning, QString("初始化数据库链接失败: %1").arg(query.lastError().text()));
    }
    // 创建 idx_conversations_updated_time 索引
    if (!query.exec(R"(
            CREATE INDEX IF NOT EXISTS idx_conversations_updated_time ON conversations(updated_time);
        )"))
    {
        XLC_LOG_WARN("Initialize database (query={}): {}", query.lastQuery(), query.lastError().text());
        ToastManager::showMessage(Toast::Type::Warning, QString("初始化数据库链接失败: %1").arg(query.lastError().text()));
    }

    // 创建 messages 表
    if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS messages (
                seq INTEGER PRIMARY KEY AUTOINCREMENT,
                id TEXT UNIQUE NOT NULL,
                conversation_id TEXT NOT NULL,
                role TEXT NOT NULL CHECK(role IN ('USER', 'ASSISTANT', 'TOOL', 'SYSTEM', 'UNKNOWN')),
                content TEXT NOT NULL,
                created_time TEXT NOT NULL,
                avatar_file_path TEXT,
                tool_calls TEXT,
                tool_call_id TEXT,

                FOREIGN KEY(conversation_id) REFERENCES conversations(id)
                    ON DELETE CASCADE
                    ON UPDATE CASCADE
            );
        )"))
    {
        XLC_LOG_WARN("Initialize database failed (query={}): {}", query.lastQuery(), query.lastError().text());
        ToastManager::showMessage(Toast::Type::Warning, QString("初始化数据库链接失败: %1").arg(query.lastError().text()));
        return;
    }
    // 创建 idx_conversations_agent_id 索引
    if (!query.exec(R"(
            CREATE INDEX IF NOT EXISTS idx_messages_conversation_id ON messages(conversation_id);
        )"))
    {
        XLC_LOG_WARN("Initialize database (query={}): {}", query.lastQuery(), query.lastError().text());
        ToastManager::showMessage(Toast::Type::Warning, QString("初始化数据库链接失败: %1").arg(query.lastError().text()));
    }
    // 创建 idx_conversations_updated_time 索引
    if (!query.exec(R"(
            CREATE INDEX IF NOT EXISTS idx_messages_created_time ON messages(created_time);
        )"))
    {
        XLC_LOG_WARN("Initialize database (query={}): {}", query.lastQuery(), query.lastError().text());
        ToastManager::showMessage(Toast::Type::Warning, QString("初始化数据库链接失败: %1").arg(query.lastError().text()));
    }
}

void DataBaseWorker::slot_initialize()
{

    m_dataBase = QSqlDatabase::addDatabase("QSQLITE", DB_CONNECTION_NAME);
    m_dataBase.setDatabaseName(m_dataBaseFile);

    if (!m_dataBase.open())
    {
        XLC_LOG_CRITICAL("Worker failed to open database: {}", m_dataBase.lastError().text());
        ToastManager::showMessage(Toast::Error, QString("未能打开数据库: %1").arg(m_dataBase.lastError().text()));
        return;
    }
    else
        XLC_LOG_INFO("Worker opened database successfully");

    initializeDatabase();
}

void DataBaseWorker::slot_getAllConversationInfo()
{
    QSqlQuery query(m_dataBase);
    query.prepare(R"(
                SELECT
                    c.id,
                    c.agent_id,
                    c.summary,
                    c.created_time,
                    c.updated_time,
                    COUNT(m.conversation_id) AS message_count
                FROM
                    conversations c
                LEFT JOIN
                    messages m ON c.id = m.conversation_id
                GROUP BY
                    c.id, c.agent_id, c.summary, c.created_time, c.updated_time
                ORDER BY
                    c.updated_time DESC,
                    c.created_time DESC
            )");
    if (!query.exec())
    {
        Q_EMIT sig_allConversationInfoAcquired(false, QJsonArray());
        XLC_LOG_WARN("Get all conversation information failed (query={}): {}", query.lastQuery(), query.lastError().text());
        ToastManager::showMessage(Toast::Type::Warning, QString("获取对话信息失败: %1").arg(query.lastError().text()));
        return;
    }
    // 解析数据
    QJsonArray jsonArrayConversationInfo;
    while (query.next())
    {
        QJsonObject jsonObjConversationInfo;
        jsonObjConversationInfo["uuid"] = query.value(0).toString();
        jsonObjConversationInfo["agent_uuid"] = query.value(1).toString();
        jsonObjConversationInfo["summary"] = query.value(2).toString();
        jsonObjConversationInfo["created_time"] = query.value(3).toString();
        jsonObjConversationInfo["updated_time"] = query.value(4).toString();
        jsonObjConversationInfo["message_count"] = query.value(5).toInt();
        jsonArrayConversationInfo.append(jsonObjConversationInfo);
    }
    XLC_LOG_TRACE("Get all conversation information successfully (conversationsCount={}, query={})", jsonArrayConversationInfo.size(), query.lastQuery());
    Q_EMIT sig_allConversationInfoAcquired(true, jsonArrayConversationInfo);
}

void DataBaseWorker::slot_insertNewConversation(const QString &agentUuid,
                                                const QString &uuid,
                                                const QString &summary,
                                                const QString &createdTime,
                                                const QString &updatedTime)
{
    QSqlQuery query(m_dataBase);
    query.prepare(R"(
                INSERT INTO conversations (id, agent_id, summary, created_time, updated_time)
                VALUES (:id, :agent_id, :summary, :created_time, :updated_time)
            )");
    query.bindValue(":id", uuid);
    query.bindValue(":agent_id", agentUuid);
    query.bindValue(":summary", summary);
    query.bindValue(":created_time", createdTime);
    query.bindValue(":updated_time", updatedTime);
    if (!query.exec())
    {
        XLC_LOG_WARN("Insert conversations failed (uuid={}, agentUuid={}, summary={}, createdTime={}, updatedTime={}, query={}): {}",
                     uuid,
                     agentUuid,
                     summary,
                     createdTime,
                     updatedTime,
                     query.lastQuery(),
                     query.lastError().text());
        ToastManager::showMessage(Toast::Type::Warning, QString("未能插入对话 (conversationUuid=%1): %2").arg(uuid).arg(query.lastError().text()));
    }
    else
    {
        XLC_LOG_TRACE("Insert conversations successfully (uuid={}, agentUuid={}, summary={}, createdTime={}, updatedTime={}, query={})",
                      uuid,
                      agentUuid,
                      summary,
                      createdTime,
                      updatedTime,
                      query.lastQuery());
    }
}

void DataBaseWorker::slot_insertNewMessage(const QString &conversationUuid,
                                           const QString &uuid,
                                           int role,
                                           const QString &content,
                                           const QString &createdTime,
                                           const QString &avatarFilePath,
                                           const QJsonArray &toolCalls,
                                           const QString &toolCallId)
{
    QString strRole;
    switch (role)
    {
    case 0:
        strRole = "USER";
        break;
    case 1:
        strRole = "ASSISTANT";
        break;
    case 2:
        strRole = "TOOL";
        break;
    case 3:
        strRole = "SYSTEM";
        break;
    default:
        strRole = "UNKNOWN";
        break;
    }
    QString strToolCalls = QString::fromUtf8(QJsonDocument(toolCalls).toJson(QJsonDocument::Indented));
    // 插入新消息
    QSqlQuery query(m_dataBase);
    query.prepare(R"(
                INSERT INTO messages (id, conversation_id, role, content, created_time, avatar_file_path, tool_calls, tool_call_id)
                VALUES (:id, :conversation_id, :role, :content, :created_time, :avatar_file_path, :tool_calls, :tool_call_id)
            )");
    query.bindValue(":id", uuid);
    query.bindValue(":conversation_id", conversationUuid);
    query.bindValue(":role", strRole);
    query.bindValue(":content", content);
    query.bindValue(":created_time", createdTime);
    query.bindValue(":avatar_file_path", avatarFilePath);
    query.bindValue(":tool_calls", strToolCalls);
    query.bindValue(":tool_call_id", toolCallId);
    if (!query.exec())
    {
        XLC_LOG_WARN("Insert message failed (uuid={}, conversationUuid={}, role={}, content={}, createdTime={}, avatarFilePath={}, toolCalls={}, toolCallId={}, query={}): {}",
                     uuid,
                     conversationUuid,
                     strRole,
                     content,
                     createdTime,
                     avatarFilePath,
                     strToolCalls,
                     toolCallId,
                     query.lastQuery(),
                     query.lastError().text());
        ToastManager::showMessage(Toast::Type::Warning, QString("未能插入消息 (uuid=%1): %2").arg(uuid).arg(query.lastError().text()));
    }
    else
    {
        XLC_LOG_TRACE("Insert message successfully (uuid={}, conversationUuid={}, role={}, content={}, createdTime={}, avatarFilePath={}, toolCalls={}, toolCallId={}, query={})",
                      uuid,
                      conversationUuid,
                      strRole,
                      content,
                      createdTime,
                      avatarFilePath,
                      strToolCalls,
                      toolCallId,
                      query.lastQuery());
    }

    // 更新对应 conversation 的更新时间
    slot_updateConversationUpdatedTime(conversationUuid, createdTime);
}

void DataBaseWorker::slot_updateConversationUpdatedTime(const QString &uuid, const QString &newUpdatedTime)
{
    // 更新 conversation 的更新时间
    QSqlQuery query(m_dataBase);
    query.prepare(R"(
                UPDATE conversations
                SET 
                updated_time = :new_updated_time
                WHERE
                id = :id
            )");
    query.bindValue(":new_updated_time", newUpdatedTime);
    query.bindValue(":id", uuid);
    if (!query.exec())
    {
        XLC_LOG_WARN("Update conversation updated_time failed (uuid={}, newUpdatedTime={}, query={}): {}",
                     uuid,
                     newUpdatedTime,
                     query.lastQuery(),
                     query.lastError().text());
        ToastManager::showMessage(Toast::Type::Warning, QString("未能更新对话最后更新时间 (conversationUuid=%1): %2").arg(uuid).arg(query.lastError().text()));
    }
    else
    {
        XLC_LOG_TRACE("Update conversation updated_time successfully (uuid={}, newUpdatedTime={}, query={})",
                      uuid,
                      newUpdatedTime,
                      query.lastQuery());
    }
}

void DataBaseWorker::slot_getMessages(const QString &conversationUuid)
{
    QSqlQuery query(m_dataBase);
    query.prepare(R"(
                SELECT
                    msg.id,
                    msg.conversation_id,
                    msg.role,
                    msg.content,
                    msg.created_time,
                    msg.avatar_file_path,
                    msg.tool_calls,
                    msg.tool_call_id
                FROM
                    messages msg
                WHERE
                    msg.conversation_id = :conversationUuid
                ORDER BY
                    msg.seq
            )");
    query.bindValue(":conversationUuid", conversationUuid);
    if (!query.exec())
    {
        XLC_LOG_WARN("Get messages failed (conversationUuid={}, query={}): {}",
                     conversationUuid,
                     query.lastQuery(),
                     query.lastError().text());
        ToastManager::showMessage(Toast::Type::Warning, QString("未能获取到历史消息 (conversationUuid=%1): %2").arg(conversationUuid).arg(query.lastError().text()));
        Q_EMIT sig_messagesAcquired(false, conversationUuid, QJsonArray());
        return;
    }
    // 解析数据
    QJsonArray jsonArrayMessages;
    while (query.next())
    {
        QJsonObject jsonObjMessage;
        jsonObjMessage["uuid"] = query.value(0).toString();
        jsonObjMessage["conversation_uuid"] = query.value(1).toString();
        jsonObjMessage["role"] = query.value(2).toString();
        jsonObjMessage["content"] = query.value(3).toString();
        jsonObjMessage["created_time"] = query.value(4).toString();
        jsonObjMessage["avatar_file_path"] = query.value(5).toString();
        jsonObjMessage["tool_calls"] = query.value(6).toString();
        jsonObjMessage["tool_call_id"] = query.value(7).toString();
        jsonArrayMessages.append(jsonObjMessage);
    }
    XLC_LOG_DEBUG("Get messages successfully (conversationUuid={}, messagesCount={}, query={})", conversationUuid, jsonArrayMessages.size(), query.lastQuery());
    Q_EMIT sig_messagesAcquired(true, conversationUuid, jsonArrayMessages);
}

void DataBaseWorker::slot_deleteConversation(const QString &conversationUuid)
{
    QSqlQuery query(m_dataBase);
    query.prepare(R"(
                DELETE
                FROM
                    conversations
                WHERE
                    id = :conversationUuid
            )");
    query.bindValue(":conversationUuid", conversationUuid);
    if (!query.exec())
    {
        XLC_LOG_WARN("Delete conversation failed (uuid={}, query={}): {}",
                     conversationUuid,
                     query.lastQuery(),
                     query.lastError().text());
        ToastManager::showMessage(Toast::Type::Warning, QString("未能删除对话 (conversationUuid=%1): %2").arg(conversationUuid).arg(query.lastError().text()));
    }
    else
    {
        XLC_LOG_TRACE("Delete conversation successfully (uuid={}, query={})",
                      conversationUuid,
                      query.lastQuery());
    }
}