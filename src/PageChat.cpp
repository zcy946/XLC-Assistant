#include "PageChat.h"
#include <QSplitter>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <FlowLayout.h>
#include "global.h"
#include "Logger.hpp"

PageChat::PageChat(QWidget *parent)
    : BaseWidget(parent)
{
    initUI();
}

void PageChat::initWidget()
{
}

void PageChat::initItems()
{
    // m_listWidgetHistory
    m_listWidgetHistory = new QListWidget(this);
#ifdef QT_DEBUG
    for (int i = 0; i < 20; ++i)
    {
        m_listWidgetHistory->addItem("测试对话记录" + QString::number(i + 1));
    }
#endif
    // m_widgetChat
    m_widgetChat = new WidgetChat(this);
}

void PageChat::initLayout()
{
    // splitter
    QSplitter *splitter = new QSplitter(this);
    splitter->setChildrenCollapsible(false);
    splitter->addWidget(m_listWidgetHistory);
    splitter->addWidget(m_widgetChat);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 8);
    // vLayout
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addWidget(splitter);
}

/**
 * WidgetChat
 */
WidgetChat::WidgetChat(QWidget *parent)
{
    initUI();
}

void WidgetChat::initWidget()
{
}

void WidgetChat::initItems()
{
    // m_listWidgetMessages
    m_listWidgetMessages = new QListWidget(this);
#ifdef QT_DEBUG
    for (int i = 0; i < 20; ++i)
    {
        m_listWidgetMessages->addItem("测试消息" + QString::number(i + 1));
    }
#endif
    // m_plainTextEdit
    m_plainTextEdit = new QPlainTextEdit(this);
    // m_pushButtonClearContext
    m_pushButtonClearContext = new QPushButton("清除上下文", this);
    connect(m_pushButtonClearContext, &QPushButton::clicked, this,
            []()
            {
                LOG_DEBUG("清除上下文");
            });
    // m_pushButtonNewChat
    m_pushButtonNewChat = new QPushButton(this);
    m_pushButtonNewChat->setText("新建对话");
    connect(m_pushButtonNewChat, &QPushButton::clicked, this,
            []()
            {
                LOG_DEBUG("新建对话");
            });
}

void WidgetChat::initLayout()
{
    // m_splitter
    QSplitter *m_splitter = new QSplitter(Qt::Orientation::Vertical, this);
    m_splitter->setChildrenCollapsible(false);
    m_splitter->addWidget(m_listWidgetMessages);
    m_splitter->addWidget(m_plainTextEdit);
    m_splitter->setStretchFactor(0, 8);
    m_splitter->setStretchFactor(1, 2);
    // flowLayoutTools
    FlowLayout *flowLayoutTools = new FlowLayout();
    flowLayoutTools->setContentsMargins(0, 0, 0, 0);
    flowLayoutTools->addWidget(m_pushButtonClearContext);
    flowLayoutTools->addWidget(m_pushButtonNewChat);
#ifdef QT_DEBUG
    for (int i = 0; i < 10; ++i)
    {
        QPushButton *button = new QPushButton("tool" + QString::number(i + 1), this);
        connect(button, &QPushButton::clicked, this,
                [i]()
                {
                    LOG_DEBUG("tool{}", i);
                });
        flowLayoutTools->addWidget(button);
    }
#endif
    // vLayout
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addWidget(m_splitter);
    vLayout->addLayout(flowLayoutTools);
}
