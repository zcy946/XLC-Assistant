#include "PageChat.h"
#include <QSplitter>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <FlowLayout.h>
#include "DataManager.h"
#include "Logger.hpp"
#include "EventBus.h"
#include "MCPService.h"
#include "LLMService.h"

PageChat::PageChat(QWidget *parent)
    : BaseWidget(parent)
{
    initUI();
    connect(DataManager::getInstance(), &DataManager::sig_agentsLoaded, this, &PageChat::slot_onAgentsLoaded);
    connect(DataManager::getInstance(), &DataManager::sig_agentUpdate, this, &PageChat::slot_onAgentUpdated);
    connect(DataManager::getInstance(), &DataManager::sig_conversationsLoaded, this, &PageChat::slot_onConversationsLoaded);
    connect(EventBus::getInstance().get(), &EventBus::sig_pageSwitched, this, &PageChat::slot_handlePageSwitched);
    connect(EventBus::getInstance().get(), &EventBus::sig_stateChanged, this, &PageChat::slot_handleStateChanged);
    connect(LLMService::getInstance(), &LLMService::sig_responseReady, this, &PageChat::slot_handleResponse);
    connect(LLMService::getInstance(), &LLMService::sig_toolCalled, this, &PageChat::slot_handleToolCalled);
    connect(m_widgetChat, &WidgetChat::sig_messageSent, this, &PageChat::slot_onMessageSent);
    connect(m_widgetChat, &WidgetChat::sig_btnClickedCreateNewConversation, this, &PageChat::slot_onBtnClickedCreateNewConversation);
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
                XLC_LOG_TRACE("Agent selected (uuid={})", uuid);
                refreshConversationList();
            });
    // m_listWidgetConversations
    m_listWidgetConversations = new QListWidget(this);
    connect(m_listWidgetConversations, &QListWidget::itemClicked, this,
            [this](QListWidgetItem *item)
            {
                const QString &uuid = item->data(Qt::UserRole).toString();
                XLC_LOG_TRACE("Conversation selected (uuid={})", uuid);
                // 刷新WidgetChat
                m_widgetChat->refreshHistoryMessageList(uuid);
            });
    // #ifdef QT_DEBUG
    //     for (int i = 0; i < 5; ++i)
    //     {
    //         // QString nameAgent = "agent实例测试" + QString::number(i + 1);
    //         // QListWidgetItem *itemAgent = new QListWidgetItem();
    //         // itemAgent->setText(nameAgent);
    //         // itemAgent->setData(Qt::UserRole, QVariant::fromValue(generateUuid());
    //         // m_listWidgetAgents->addItem(itemAgent);
    //         if (!DataManager::getInstance()->getAgents().isEmpty())
    //         {
    //             std::shared_ptr<Agent> agent = DataManager::getInstance()->getAgents().first();
    //             std::shared_ptr<Conversation> newConversation = DataManager::getInstance()->createNewConversation(agent->uuid);
    //             if (newConversation)
    //             {
    //                 newConversation->summary = "对话实例测试" + QString::number(i + 1);
    //                 QListWidgetItem *itemNewConversation = new QListWidgetItem();
    //                 itemNewConversation->setText(newConversation->summary);
    //                 itemNewConversation->setData(Qt::UserRole, QVariant::fromValue(newConversation->uuid));
    //                 m_listWidgetConversations->addItem(itemNewConversation);
    //                 DataManager::getInstance()->addConversation(newConversation);
    //             }
    //         }
    //     }
    //     m_listWidgetConversations->sortItems();
    // #endif
    if (m_listWidgetConversations->currentItem() == nullptr)
    {
        m_listWidgetConversations->setCurrentRow(0);
    }

    // m_tabWidgetSiderBar
    m_tabWidgetSiderBar = new QTabWidget(this);
    m_tabWidgetSiderBar->addTab(m_listWidgetAgents, "智能体");
    m_tabWidgetSiderBar->addTab(m_listWidgetConversations, "话题");
    // 默认选中 话题
    m_tabWidgetSiderBar->setCurrentWidget(m_listWidgetConversations);
    connect(m_tabWidgetSiderBar, &QTabWidget::currentChanged, this,
            [this](int index)
            {
                XLC_LOG_TRACE("Switched tab (index={}, text={})", index, m_tabWidgetSiderBar->tabText(index));
            });
    // m_widgetChat
    QListWidgetItem *itemConversation = m_listWidgetConversations->currentItem();
    if (itemConversation)
        m_widgetChat = new WidgetChat(itemConversation->data(Qt::UserRole).toString(), this);
    else
        m_widgetChat = new WidgetChat(Q_NULLPTR, this);
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
    refreshAgentList();
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
    refreshAgentList();
    m_listWidgetAgents->sortItems();
    // 更新conversations列表
    QListWidgetItem *currentItem = m_listWidgetAgents->currentItem();
    if (!currentItem)
        return;
    if (currentItem->data(Qt::UserRole).toString() == agentUuid)
    {
        refreshConversationList();
    }
}

void PageChat::slot_onConversationsLoaded(bool success)
{
    if (!success)
        return;
    refreshConversationList();
    m_listWidgetConversations->sortItems();
    // 默认选中并展示第一项
    if (m_listWidgetConversations->currentItem() == nullptr)
    {
        m_listWidgetConversations->setCurrentRow(0);
    }
}

void PageChat::slot_onMessageSent(const QString &message)
{
    QListWidgetItem *itemSelectedAgent = m_listWidgetAgents->currentItem();
    if (!itemSelectedAgent)
    {
        XLC_LOG_WARN("Send message failed : select or create a agent");
        return;
    }
    QString agentUuid = itemSelectedAgent->data(Qt::UserRole).toString();
    const std::shared_ptr<Agent> &agent = DataManager::getInstance()->getAgent(agentUuid);
    if (!agent)
    {
        XLC_LOG_WARN("Send message failed (agentUuid={}): agent not found", agentUuid);
        return;
    }
    QListWidgetItem *itemSelectedConversation = m_listWidgetConversations->currentItem();
    if (!itemSelectedConversation)
    {
        XLC_LOG_WARN("Send message failed (agentUuid={}): select or create a conversation", agentUuid);
        return;
    }
    QString conversationUuid = itemSelectedConversation->data(Qt::UserRole).toString();
    const std::shared_ptr<Conversation> &conversation = DataManager::getInstance()->getConversation(conversationUuid);
    if (!conversation)
    {
        XLC_LOG_WARN("Send message failed (agentUuid={}, conversationUuid={}): conversation not found", agentUuid, conversationUuid);
        return;
    }
    // 检查 MCP 服务器是否初始化
    bool allMcpServersReady = true;
    for (const QString &mcpServerUuid : agent->mcpServers)
    {
        if (!MCPService::getInstance()->isInitialized(mcpServerUuid))
        {
            allMcpServersReady = false;

            // 初始化未初始化的MCP服务器
            MCPService::getInstance()->initClient(mcpServerUuid);
        }
    }
    if (allMcpServersReady)
    {
        m_widgetChat->addNewMessage(CMessage(message, Message::Role::USER));
        // 记录问题
        conversation->addMessage({{"role", "user"}, {"content", message.toStdString()}});
        LLMService::getInstance()->processRequest(conversation, agent, MCPService::getInstance()->getToolsFromServers(agent->mcpServers));
    }
    else
    {
        XLC_LOG_WARN("Send message failed (agentUuid={}, conversationUuid={}): MCP servers not yet all initialized ", agentUuid, conversationUuid);
    }
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
                XLC_LOG_ERROR("Switch failed(agentUuid={}, conversationUuid={}): agent or conversation UUID is empty ", agentUuid, conversationUuid);
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
                    XLC_LOG_WARN("Switch failed (conversationUuid={}): agent found but conversation not found", conversationUuid);
                    break;
                }
            }
            XLC_LOG_WARN("Switch failed (agentUuid={}): agent not found", agentUuid);
            break;
        }
        default:
        {
            XLC_LOG_ERROR("Switch failed ({}): page ID not found", id);
            break;
        }
        }
    }
    else
    {
        XLC_LOG_ERROR("Failed to process page switch event (type={}): data type abnormal", data.typeName());
    }
}

void PageChat::slot_handleStateChanged(const QVariant &data)
{
    if (data.canConvert<QJsonObject>())
    {
        QJsonObject jsonObj = data.value<QJsonObject>();
        int id = jsonObj["id"].toInt();

        switch (static_cast<EventBus::States>(id))
        {
        case EventBus::States::AGENT_UPDATED:
        {
            refreshAgentList();
            break;
        }
        default:
        {
            break;
        }
        }
    }
    else
    {
        XLC_LOG_ERROR("Failed to process status change event(typeName={}): data type exception", data.typeName());
    }
}

void PageChat::slot_handleResponse(const QString &conversationUuid, const QString &responseMessage)
{
    if (m_widgetChat->getConversationUuid() == conversationUuid)
    {
        m_widgetChat->addNewMessage(CMessage(responseMessage, Message::Role::ASSISTANT));
    }
}

void PageChat::slot_handleToolCalled(const QString &conversationUuid, const QString &message)
{
    if (m_widgetChat->getConversationUuid() == conversationUuid)
    {
        m_widgetChat->addNewMessage(CMessage(message, Message::Role::TOOL));
    }
}

void PageChat::slot_onBtnClickedCreateNewConversation()
{
    QListWidgetItem *currentSelectedAgentItem = m_listWidgetAgents->currentItem();
    if (!currentSelectedAgentItem)
    {
        XLC_LOG_WARN("Create new conversation failed: no selected agent item");
        return;
    }
    QString agentUuid = currentSelectedAgentItem->data(Qt::UserRole).toString();
    std::shared_ptr<Agent> agent = DataManager::getInstance()->getAgent(agentUuid);
    if (!agent)
    {
        XLC_LOG_WARN("Create new conversation failed (agentUuid={}): agent not found", agentUuid);
        return;
    }
    std::shared_ptr<Conversation> newConversation = DataManager::getInstance()->createNewConversation(agentUuid);
    if (newConversation)
    {
        newConversation->summary = QString("新建对话 - %1").arg(getCurrentDateTime());
        QListWidgetItem *itemNewConversation = new QListWidgetItem();
        itemNewConversation->setText(newConversation->summary);
        itemNewConversation->setData(Qt::UserRole, QVariant::fromValue(newConversation->uuid));
        m_listWidgetConversations->addItem(itemNewConversation);
        DataManager::getInstance()->addConversation(newConversation);
        // 选择新建的对话
        m_listWidgetConversations->setCurrentItem(itemNewConversation);
        // 刷新WidgetChat
        m_widgetChat->refreshHistoryMessageList(newConversation->uuid);
        XLC_LOG_INFO("Create new conversation successfully");
    }
}

void PageChat::refreshAgentList()
{
    // 保留当前选中agent的uuid，用于再次选中
    QString selectedAgentUuid = -1;
    QListWidgetItem *selectedAgentItem = m_listWidgetAgents->currentItem();
    if (selectedAgentItem && DataManager::getInstance()->getAgent(selectedAgentItem->data(Qt::UserRole).toString()))
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
    XLC_LOG_DEBUG("Agent already deleted, cannot be selected (selectedAgentUuid={})", selectedAgentUuid);
}

void PageChat::refreshConversationList()
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
        XLC_LOG_DEBUG("No agent selected, no need to update conversations list.");
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
    XLC_LOG_DEBUG("Conversation has been deleted and cannot be selected; first item selected by default (selectedConversationUuid={})", selectedConversationUuid);
}

/**
 * WidgetChat
 */
WidgetChat::WidgetChat(const QString &conversationUuid, QWidget *parent)
    : BaseWidget(parent), m_conversationUuid(conversationUuid)
{
    initUI();
}

void WidgetChat::initWidget()
{
}

void WidgetChat::initItems()
{
    // m_listWidgetMessages
    m_listWidgetMessages = new CMessageListWidget(this);
#ifdef QT_DEBUG
    for (int i = 0; i < 50; ++i)
    {
        if (i % 2 == 0)
            m_listWidgetMessages->addMessage(CMessage("测试消息" + QString::number(i + 1), Message::Role::USER));
        else
            m_listWidgetMessages->addMessage(CMessage("测试消息" + QString::number(i + 1), Message::Role::ASSISTANT));
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
            [this]()
            {
                std::shared_ptr<Conversation> conversation = DataManager::getInstance()->getConversation(m_conversationUuid);
                if (!conversation)
                {
                    XLC_LOG_WARN("Clear context failed (conversationUuid={}): conversation not found", m_conversationUuid);
                    return;
                }
                conversation->clearContext();
                XLC_LOG_INFO("Clear context successed");
            });
    // m_pushButtonCreateNewConversation
    m_pushButtonCreateNewConversation = new QPushButton(this);
    m_pushButtonCreateNewConversation->setText("新建对话");
    connect(m_pushButtonCreateNewConversation, &QPushButton::clicked, this, &WidgetChat::sig_btnClickedCreateNewConversation);
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
    flowLayoutTools->addWidget(m_pushButtonCreateNewConversation);
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

void WidgetChat::addNewMessage(CMessage message)
{
    m_listWidgetMessages->addMessage(message);
    m_listWidgetMessages->scrollToBottom();
}

const QString WidgetChat::getConversationUuid()
{
    return m_conversationUuid;
}

void WidgetChat::refreshHistoryMessageList(const QString &newConversationUuid)
{
    if (newConversationUuid.trimmed().isEmpty())
    {
        XLC_LOG_WARN("Refresh history message list failed (conversationUuid={}): empty uuid", newConversationUuid);
        return;
    }
    std::shared_ptr<Conversation> conversation = DataManager::getInstance()->getConversation(newConversationUuid);
    if (!conversation)
    {
        XLC_LOG_WARN("Refresh history message list failed (conversationUuid={}): conversation not found", newConversationUuid);
        return;
    }

    // 刷新缓存uuid
    m_conversationUuid = newConversationUuid;

    // 刷新消息列表
    mcp::json messages = conversation->getMessages();
}