#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include "Singleton.h"
#include <QSqlDatabase>
#include <QThread>
#include <QJsonArray>

class DataBaseWorker;
class DataBaseManager : public QObject, public Singleton<DataBaseManager>
{
    Q_OBJECT
    friend class Singleton<DataBaseManager>;

Q_SIGNALS:
    // 获取所有对话信息
    void sig_getAllConversationInfo();
    // 新增对话
    void sig_insertNewConversation(const QString &agentUuid,
                                   const QString &uuid,
                                   const QString &summary,
                                   const QString &createdTime,
                                   const QString &updatedTime);
    // 新增消息
    void sig_insertNewMessage(const QString &conversationUuid,
                              const QString &uuid,
                              int role,
                              const QString &text,
                              const QString &createdTime,
                              const QString &avatarFilePath,
                              const QString &toolCalls,
                              const QString &toolCallId);
    // 获取消息列表
    void sig_getMessageList(const QString &conversationUuid);
    // 删除对话
    void sig_deleteConversation(const QString &conversationUuid);

public:
    ~DataBaseManager();
    const DataBaseWorker *getWorkerPtr();

private:
    explicit DataBaseManager(QObject *parent = nullptr);

private:
    DataBaseWorker *m_worker;
    QThread m_thread;
    const QString DATABASE_FILENAME = "xlc_assistant.db";
};

class DataBaseWorker : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void sig_allConversationInfoAcquired(bool success, QJsonArray jsonArrayConversationInfo);
    void sig_messagesAcquired(bool success, const QString &conversationUuid, QJsonArray jsonArrayMessages);

public Q_SLOTS:
    void slot_initialize();
    void slot_getAllConversationInfo();
    void slot_insertNewConversation(const QString &agentUuid,
                                    const QString &uuid,
                                    const QString &summary,
                                    const QString &createdTime,
                                    const QString &updatedTime);
    void slot_insertNewMessage(const QString &conversationUuid,
                               const QString &uuid,
                               int role,
                               const QString &content,
                               const QString &createdTime,
                               const QString &avatarFilePath,
                               const QString &toolCalls,
                               const QString &toolCallId);
    void slot_updateConversationUpdatedTime(const QString &uuid, const QString &newUpdatedTime);
    void slot_getMessages(const QString &conversationUuid);
    void slot_deleteConversation(const QString &conversationUuid);

public:
    explicit DataBaseWorker(const QString &dataBaseFile, QObject *parent = nullptr);
    ~DataBaseWorker();

private:
    void initializeDatabase();

private:
    QSqlDatabase m_dataBase;
    QString m_dataBaseFile;
    const QString DB_CONNECTION_NAME = "databaseworker_connection";
};

#endif // DATABASEMANAGER_H

/**
 * NOTE QSqlDatabase 的局部特性
 * QSqlDatabase具有局部特写，在使用的时候要确保执行query的线程和打开的数据库连接QSqlDatabase处于同一线程中。
 *  */