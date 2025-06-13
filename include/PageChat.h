#ifndef PAGECHAT_H
#define PAGECHAT_H

#include "BaseWidget.hpp"
#include <QListWidget>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>

class WidgetChat;
class PageChat : public BaseWidget
{
    Q_OBJECT
public:
    explicit PageChat(QWidget *parent = nullptr);

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;

private:
    QListWidget *m_listWidgetHistory;
    WidgetChat *m_widgetChat;
};

class WidgetChat : public BaseWidget
{
    Q_OBJECT
public:
    explicit WidgetChat(QWidget *parent = nullptr);

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;

private:
    QListWidget *m_listWidgetMessages;
    QPlainTextEdit *m_plainTextEdit;
    QPushButton *m_pushButtonClearContext;
    QPushButton *m_pushButtonNewChat;
};

#endif // PAGECHAT_H