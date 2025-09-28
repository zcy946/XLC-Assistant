#include "DataBaseManager.h"
#include <QCoreApplication>
#include "Logger.hpp"
#include <QSqlError>
#include <QSqlQuery>
#include <QtConcurrent>

DataBaseManager::DataBaseManager(QObject *parent)
    : QObject(parent)
{
    // 创建数据库线程
    m_worker = new DataBaseWorker(DATABASE_FILENAME);
    m_worker->moveToThread(&m_thread);

    connect(qApp, &QCoreApplication::aboutToQuit, &m_thread, &QThread::quit);

    // 在线程启动后初始化数据库，确保 QSqlDatabase 和 worker 在同一线程
    connect(&m_thread, &QThread::started, m_worker, &DataBaseWorker::slot_initialize);
    connect(this, &DataBaseManager::sig_insertNewMessage, m_worker, &DataBaseWorker::slot_insertNewMessage, Qt::QueuedConnection);

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
        XLC_LOG_ERROR("Initialize database failed (query={}): {}", query.lastQuery(), query.lastError().text());
        return;
    }
    // 创建 idx_conversations_agent_id 索引
    if (!query.exec(R"(
            CREATE INDEX IF NOT EXISTS idx_conversations_agent_id ON conversations(agent_id);
        )"))
    {
        XLC_LOG_WARN("Initialize database (query={}): {}", query.lastQuery(), query.lastError().text());
    }
    // 创建 idx_conversations_updated_time 索引
    if (!query.exec(R"(
            CREATE INDEX IF NOT EXISTS idx_conversations_updated_time ON conversations(updated_time);
        )"))
    {
        XLC_LOG_WARN("Initialize database (query={}): {}", query.lastQuery(), query.lastError().text());
    }

    // 创建 messages 表
    if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS messages (
                seq INTEGER PRIMARY KEY AUTOINCREMENT,
                id TEXT UNIQUE NOT NULL,
                conversation_id TEXT NOT NULL,
                role TEXT NOT NULL CHECK(role IN ('USER', 'ASSISTANT', 'TOOL', 'SYSTEM', 'UNKNOWN')),
                text TEXT NOT NULL,
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
        XLC_LOG_ERROR("Initialize database failed (query={}): {}", query.lastQuery(), query.lastError().text());
        return;
    }
    // 创建 idx_conversations_agent_id 索引
    if (!query.exec(R"(
            CREATE INDEX idx_messages_conversation_id ON messages(conversation_id);
        )"))
    {
        XLC_LOG_WARN("Initialize database (query={}): {}", query.lastQuery(), query.lastError().text());
    }
    // 创建 idx_conversations_updated_time 索引
    if (!query.exec(R"(
            CREATE INDEX idx_messages_created_time ON messages(created_time);
        )"))
    {
        XLC_LOG_WARN("Initialize database (query={}): {}", query.lastQuery(), query.lastError().text());
    }
}

void DataBaseWorker::slot_initialize()
{

    m_dataBase = QSqlDatabase::addDatabase("QSQLITE", DB_CONNECTION_NAME);
    m_dataBase.setDatabaseName(m_dataBaseFile);

    if (!m_dataBase.open())
    {
        XLC_LOG_CRITICAL("Worker failed to open database: {}", m_dataBase.lastError().text());
        return;
    }
    else
        XLC_LOG_INFO("Worker opened database successfully");

    initializeDatabase();
}

void DataBaseWorker::slot_insertNewMessage(const QString &conversationUuid,
                                           const QString &uuid,
                                           int role,
                                           const QString &text,
                                           const QString &createdTime,
                                           const QString &avatarFilePath,
                                           const QString &toolCalls,
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
    QSqlQuery query(m_dataBase);
    query.prepare(R"(
                INSERT INTO messages (id, conversation_id, role, text, created_time, avatar_file_path, tool_calls, tool_call_id)
                VALUES (:id, :conversation_id, :role, :text, :created_time, :avatar_file_path, :tool_calls, :tool_call_id)
            )");
    query.bindValue(":id", uuid);
    query.bindValue(":conversation_id", conversationUuid);
    query.bindValue(":role", strRole);
    query.bindValue(":text", text);
    query.bindValue(":created_time", createdTime);
    query.bindValue(":avatar_file_path", avatarFilePath);
    query.bindValue(":tool_calls", toolCalls);
    query.bindValue(":tool_call_id", toolCallId);
    if (!query.exec())
    {
        XLC_LOG_WARN("Insert message failed (query={}): {}", query.lastQuery(), query.lastError().text());
    }
    else
    {
        XLC_LOG_DEBUG("Insert message successfully (query={})", query.lastQuery());
    }
}