#include "PageChat.h"
#include <QSplitter>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <FlowLayout.h>
#include "DataManager.h"
#include "Logger.hpp"

PageChat::PageChat(QWidget *parent)
    : BaseWidget(parent)
{
    initUI();
    connect(DataManager::getInstance(), &DataManager::sig_agentsLoaded, this, &PageChat::slot_onAgentsLoaded);
    connect(m_widgetChat, &WidgetChat::sig_messageSent, this, &PageChat::slot_onMessageSent);
}

void PageChat::initWidget()
{
}

void PageChat::initItems()
{
    // m_listWidgetAgents
    m_listWidgetAgents = new QListWidget(this);
    connect(m_listWidgetAgents, &QListWidget::itemClicked, this,
            [](QListWidgetItem *item)
            {
                const QString &uuid = item->data(Qt::UserRole).toString();
                XLC_LOG_TRACE("选中agent: {}", uuid);
            });
    // m_listWidgetConversations
    m_listWidgetConversations = new QListWidget(this);
    connect(m_listWidgetConversations, &QListWidget::itemClicked, this,
            [](QListWidgetItem *item)
            {
                const QString &uuid = item->data(Qt::UserRole).toString();
                XLC_LOG_TRACE("选中对话: {}", uuid);
            });
#ifdef QT_DEBUG
    for (int i = 0; i < 50; ++i)
    {
        // QString nameAgent = "agent实例测试" + QString::number(i + 1);
        // QListWidgetItem *itemAgent = new QListWidgetItem();
        // itemAgent->setText(nameAgent);
        // itemAgent->setData(Qt::UserRole, QVariant::fromValue(generateUuid());
        // m_listWidgetAgents->addItem(itemAgent);

        std::shared_ptr<Conversation> conversation = std::make_shared<Conversation>(DataManager::getInstance()->getAgents().first()->uuid);
        QString nameConversation = "对话实例测试" + QString::number(i + 1);
        QListWidgetItem *itemConversation = new QListWidgetItem();
        itemConversation->setText(nameConversation);
        itemConversation->setData(Qt::UserRole, QVariant::fromValue(conversation->uuid));
        m_listWidgetConversations->addItem(itemConversation);
        DataManager::getInstance()->addConversation(conversation);
    }
    m_listWidgetConversations->sortItems();
    if (m_listWidgetConversations->currentItem() == nullptr)
    {
        m_listWidgetConversations->setCurrentRow(0);
    }
#endif
    // m_tabWidgetSiderBar
    m_tabWidgetSiderBar = new QTabWidget(this);
    m_tabWidgetSiderBar->addTab(m_listWidgetAgents, "助手");
    m_tabWidgetSiderBar->addTab(m_listWidgetConversations, "话题");
    // m_widgetChat
    m_widgetChat = new WidgetChat(this);
}

void PageChat::initLayout()
{
    // splitter
    QSplitter *splitter = new QSplitter(this);
    splitter->setChildrenCollapsible(false);
    splitter->addWidget(m_tabWidgetSiderBar);
    splitter->addWidget(m_widgetChat);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 8);
    // vLayout
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addWidget(splitter);
}

void PageChat::slot_onAgentsLoaded(bool success)
{
    if (!success)
        return;
    for (auto &agent : DataManager::getInstance()->getAgents())
    {
        QListWidgetItem *itemAgent = new QListWidgetItem();
        itemAgent->setText(agent->name);
        itemAgent->setData(Qt::UserRole, QVariant::fromValue(agent->uuid));
        m_listWidgetAgents->addItem(itemAgent);
    }
    m_listWidgetAgents->sortItems();
    // 默认选中并展示第一项
    if (m_listWidgetAgents->currentItem() == nullptr)
    {
        m_listWidgetAgents->setCurrentRow(0);
    }
}

void PageChat::slot_onMessageSent(const QString &message)
{
    const std::shared_ptr<Agent> &agent = DataManager::getInstance()->getAgent(m_listWidgetAgents->currentItem()->data(Qt::UserRole).toString());
    if (!agent)
    {
        XLC_LOG_WARN("不存在的agent: {}", m_listWidgetAgents->currentItem()->data(Qt::UserRole).toString());
        return;
    }
    const std::shared_ptr<Conversation> &conversation = DataManager::getInstance()->getConversation(m_listWidgetConversations->currentItem()->data(Qt::UserRole).toString());
    if (!conversation)
    {
        XLC_LOG_WARN("不存在的conversation: {}", m_listWidgetConversations->currentItem()->data(Qt::UserRole).toString());
        return;
    }
    conversation->messages.push_back({{"role", "user"}, {"content", message.toStdString()}});
    DataManager::getInstance()->handleMessageSent(conversation, agent, DataManager::getInstance()->getTools(agent->mcpServers));
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
    for (int i = 0; i < 50; ++i)
    {
        m_listWidgetMessages->addItem("测试消息" + QString::number(i + 1));
    }
#endif
    // m_plainTextEdit
    m_plainTextEdit = new QPlainTextEdit(this);
    // m_pushButtonSend
    m_pushButtonSend = new QPushButton(this);
    m_pushButtonSend->setText("发送");
    connect(m_pushButtonSend, &QPushButton::clicked, this,
            [this]()
            {
                const QString userInput = this->m_plainTextEdit->toPlainText();
                if (!userInput.isEmpty())
                {
                    XLC_LOG_DEBUG("发送消息: {}", userInput);
                    Q_EMIT sig_messageSent(userInput);
                }
            });
    // m_pushButtonClearContext
    m_pushButtonClearContext = new QPushButton("清除上下文", this);
    connect(m_pushButtonClearContext, &QPushButton::clicked, this,
            []()
            {
                XLC_LOG_DEBUG("清除上下文");
            });
    // m_pushButtonNewChat
    m_pushButtonNewChat = new QPushButton(this);
    m_pushButtonNewChat->setText("新建对话");
    connect(m_pushButtonNewChat, &QPushButton::clicked, this,
            []()
            {
                XLC_LOG_DEBUG("新建对话");
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
                    XLC_LOG_DEBUG("tool{}", i);
                });
        flowLayoutTools->addWidget(button);
    }
#endif
    // hLayoutTools
    QHBoxLayout *hLayoutTools = new QHBoxLayout();
    hLayoutTools->setContentsMargins(0, 0, 0, 0);
    hLayoutTools->addLayout(flowLayoutTools, 1);
    hLayoutTools->addWidget(m_pushButtonSend, 0);
    // vLayout
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addWidget(m_splitter);
    vLayout->addLayout(hLayoutTools);
}