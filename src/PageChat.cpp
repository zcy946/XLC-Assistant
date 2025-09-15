#include "PageChat.h"
#include <QSplitter>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <FlowLayout.h>
#include "DataManager.h"
#include "Logger.hpp"
#include "EventBus.h"

PageChat::PageChat(QWidget *parent)
    : BaseWidget(parent)
{
    initUI();
    connect(DataManager::getInstance(), &DataManager::sig_agentsLoaded, this, &PageChat::slot_onAgentsLoaded);
    connect(m_widgetChat, &WidgetChat::sig_messageSent, this, &PageChat::slot_onMessageSent);
    connect(DataManager::getInstance(), &DataManager::sig_agentUpdate, this, &PageChat::slot_onAgentUpdated);
    connect(EventBus::GetInstance().get(), &EventBus::sig_pageSwitched, this, &PageChat::slot_handlePageSwitched);
}

void PageChat::initWidget()
{
}

void PageChat::initItems()
{
    // m_listWidgetAgents
    m_listWidgetAgents = new QListWidget(this);
    connect(m_listWidgetAgents, &QListWidget::itemClicked, this,
            [this](QListWidgetItem *item)
            {
                const QString &uuid = item->data(Qt::UserRole).toString();
                XLC_LOG_TRACE("选中agent: {}", uuid);
                refreshConversations();
            });
    // m_listWidgetConversations
    m_listWidgetConversations = new QListWidget(this);
    connect(m_listWidgetConversations, &QListWidget::itemClicked, this,
            [](QListWidgetItem *item)
            {
                const QString &uuid = item->data(Qt::UserRole).toString();
                XLC_LOG_TRACE("选中对话: {}", uuid);
                // TODO 刷新WidgetChat
            });
#ifdef QT_DEBUG
    for (int i = 0; i < 50; ++i)
    {
        // QString nameAgent = "agent实例测试" + QString::number(i + 1);
        // QListWidgetItem *itemAgent = new QListWidgetItem();
        // itemAgent->setText(nameAgent);
        // itemAgent->setData(Qt::UserRole, QVariant::fromValue(generateUuid());
        // m_listWidgetAgents->addItem(itemAgent);

        std::shared_ptr<Conversation> newConversation = DataManager::getInstance()->createNewConversation(DataManager::getInstance()->getAgents().first()->uuid);
        if (newConversation)
        {
            newConversation->summary = "对话实例测试" + QString::number(i + 1);
            QListWidgetItem *itemConversation = new QListWidgetItem();
            itemConversation->setText(newConversation->summary);
            itemConversation->setData(Qt::UserRole, QVariant::fromValue(newConversation->uuid));
            m_listWidgetConversations->addItem(itemConversation);
            DataManager::getInstance()->addConversation(newConversation);
        }
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
    // 默认选中 话题
    m_tabWidgetSiderBar->setCurrentWidget(m_listWidgetConversations);
    connect(m_tabWidgetSiderBar, &QTabWidget::currentChanged, this,
            [this](int index)
            {
                XLC_LOG_TRACE("切换至: {} - {}", index, m_tabWidgetSiderBar->tabText(index));
            });
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
    refreshAgents();
    m_listWidgetAgents->sortItems();
    // 默认选中并展示第一项
    if (m_listWidgetAgents->currentItem() == nullptr)
    {
        m_listWidgetAgents->setCurrentRow(0);
    }
}

void PageChat::slot_onAgentUpdated(const QString &agentUuid)
{
    // 更新agents列表
    refreshAgents();
    m_listWidgetAgents->sortItems();
    // 更新conversations列表
    QListWidgetItem *currentItem = m_listWidgetAgents->currentItem();
    if (!currentItem)
        return;
    if (currentItem->data(Qt::UserRole).toString() == agentUuid)
    {
        refreshConversations();
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

void PageChat::slot_handlePageSwitched(const QVariant &data)
{
    if (data.canConvert<QJsonObject>())
    {
        QJsonObject objPageInfo = data.value<QJsonObject>();
        int id = objPageInfo["id"].toInt();
        QString agentUuid = objPageInfo["agentUuid"].toString();
        QString conversationUuid = objPageInfo["conversationUuid"].toString();

        switch (static_cast<EventBus::Pages>(id))
        {
        case EventBus::Pages::CONVERSATION:
        {
            if (agentUuid.isEmpty() || conversationUuid.isEmpty())
            {
                XLC_LOG_ERROR("切换失败，agent或者conversation的uuid为空");
                return;
            }
            // 选中对话列表
            m_tabWidgetSiderBar->setCurrentWidget(m_listWidgetConversations);

            for (int i = 0; i < m_listWidgetAgents->count(); ++i)
            {
                // 查找应当选中的agent
                if (m_listWidgetAgents->item(i)->data(Qt::UserRole).toString() == agentUuid)
                {
                    m_listWidgetAgents->setCurrentRow(i);
                    // 查找应当选中的conversation
                    for (int j = 0; i < m_listWidgetConversations->count(); ++j)
                    {
                        if (m_listWidgetConversations->item(j)->data(Qt::UserRole).toString() == conversationUuid)
                        {
                            m_listWidgetConversations->setCurrentRow(j);
                            return;
                        }
                    }
                    XLC_LOG_WARN("找到了agent但未找到conversation: {}", conversationUuid);
                    break;
                }
            }
            XLC_LOG_WARN("未找到agent: {}", agentUuid);
            break;
        }
        default:
        {
            XLC_LOG_ERROR("不存在的Page id: {}", id);
            break;
        }
        }
    }
    else
    {
        XLC_LOG_ERROR("切换失败，数据类型异常: {}", data.typeName());
    }
}

void PageChat::refreshAgents()
{
    // 保留当前选中agent的uuid，用于再次选中
    QString selectedAgentUuid = -1;
    QListWidgetItem *selectedAgentItem = m_listWidgetAgents->currentItem();
    if (selectedAgentItem)
    {
        selectedAgentUuid = selectedAgentItem->data(Qt::UserRole).toString();
    }
    // 更新listwidget
    m_listWidgetAgents->clear();
    for (auto &agent : DataManager::getInstance()->getAgents())
    {
        QListWidgetItem *itemAgent = new QListWidgetItem();
        itemAgent->setText(agent->name);
        itemAgent->setData(Qt::UserRole, QVariant::fromValue(agent->uuid));
        m_listWidgetAgents->addItem(itemAgent);
    }
    // 重新选中之前的item
    if (selectedAgentUuid == -1)
        return;
    for (int i = 0; i < m_listWidgetAgents->count(); ++i)
    {
        QListWidgetItem *item = m_listWidgetAgents->item(i);
        if (item)
        {
            if (item->data(Qt::UserRole).toString() == selectedAgentUuid)
            {
                m_listWidgetAgents->setCurrentItem(item);
                return;
            }
        }
    }
    m_listWidgetAgents->setCurrentRow(0);
    XLC_LOG_DEBUG("agent: [{}] 已被删除，无法选中，已默认选中第一项", selectedAgentUuid);
}

void PageChat::refreshConversations()
{
    // 保留当前选中conversation的uuid，用于再次选中
    QString selectedConversationUuid = -1;
    QListWidgetItem *selectedConversationItem = m_listWidgetConversations->currentItem();
    if (selectedConversationItem)
    {
        selectedConversationUuid = selectedConversationItem->data(Qt::UserRole).toString();
    }
    // 更新对话列表
    QListWidgetItem *selectedAgentItem = m_listWidgetAgents->currentItem();
    if (!selectedAgentItem)
    {
        XLC_LOG_DEBUG("没有选中任何agent，无需更新conversations列表");
        return;
    }
    std::shared_ptr<Agent> currentAgent = DataManager::getInstance()->getAgent(selectedAgentItem->data(Qt::UserRole).toString());
    if (!currentAgent)
        return;
    m_listWidgetConversations->clear();
    for (const QString &uuid : currentAgent->conversations)
    {
        std::shared_ptr<Conversation> conversation = DataManager::getInstance()->getConversation(uuid);
        if (!conversation)
            continue;
        QListWidgetItem *itemConversation = new QListWidgetItem();
        itemConversation->setText(conversation->summary);
        itemConversation->setData(Qt::UserRole, QVariant::fromValue(uuid));
        m_listWidgetConversations->addItem(itemConversation);
    }
    // 重新选中之前的item
    if (selectedConversationUuid == -1)
        return;
    for (int i = 0; i < m_listWidgetConversations->count(); ++i)
    {
        QListWidgetItem *item = m_listWidgetConversations->item(i);
        if (item)
        {
            if (item->data(Qt::UserRole).toString() == selectedConversationUuid)
            {
                m_listWidgetConversations->setCurrentItem(item);
                return;
            }
        }
    }
    m_listWidgetConversations->setCurrentRow(0);
    XLC_LOG_DEBUG("conversation: [{}] 已被删除，无法选中，已默认选中第一项", selectedConversationUuid);
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