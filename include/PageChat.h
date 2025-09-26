#ifndef PAGECHAT_H
#define PAGECHAT_H

#include "BaseWidget.hpp"
#include <QListWidget>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTabWidget>
#include "CMessageListWidget.h"

class WidgetChat;
class PageChat : public BaseWidget
{
    Q_OBJECT
private Q_SLOTS:
    void slot_onAgentsLoaded(bool success);
    void slot_onAgentUpdated(const QString &agentUuid);
    // 用户点击发送按钮
    void slot_onMessageSent(const QString &message);
    void slot_handlePageSwitched(const QVariant &data);
    void slot_handleStateChanged(const QVariant &data);

public:
    explicit PageChat(QWidget *parent = nullptr);

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;

private:
    QListWidget *m_listWidgetAgents;
    QListWidget *m_listWidgetConversations;
    QTabWidget *m_tabWidgetSiderBar;
    WidgetChat *m_widgetChat;

private:
    void refreshAgents();
    void refreshConversations();
};

class WidgetChat : public BaseWidget
{
    Q_OBJECT
Q_SIGNALS:
    void sig_messageSent(const QString &message);

public:
    explicit WidgetChat(const QString &conversationUuid, QWidget *parent = nullptr);

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;

private:
    const QString m_conversationUuid;

private:
    CMessageListWidget *m_listWidgetMessages;
    QPlainTextEdit *m_plainTextEdit;
    QPushButton *m_pushButtonSend;
    QPushButton *m_pushButtonClearContext;
    QPushButton *m_pushButtonNewChat;
};

#endif // PAGECHAT_H