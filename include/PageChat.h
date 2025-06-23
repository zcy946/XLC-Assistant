#ifndef PAGECHAT_H
#define PAGECHAT_H

#include "BaseWidget.hpp"
#include <QListWidget>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTabWidget>

class WidgetChat;
class PageChat : public BaseWidget
{
    Q_OBJECT
private Q_SLOTS:
    void slot_onAgentsLoaded(bool success);
    // 用户点击发送按钮
    void slot_onMessageSent(const QString &message);

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
};

class WidgetChat : public BaseWidget
{
    Q_OBJECT
Q_SIGNALS:
    void sig_messageSent(const QString &message);

public:
    explicit WidgetChat(QWidget *parent = nullptr);

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;

private:
    QListWidget *m_listWidgetMessages;
    QPlainTextEdit *m_plainTextEdit;
    QPushButton *m_pushButtonSend;
    QPushButton *m_pushButtonClearContext;
    QPushButton *m_pushButtonNewChat;
};

#endif // PAGECHAT_H