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
}

void PageChat::initWidget()
{
}

void PageChat::initItems()
{
    // m_listWidgetAgent
    m_listWidgetAgent = new QListWidget(this);
    connect(m_listWidgetAgent, &QListWidget::itemClicked, this,
            [](QListWidgetItem *item)
            {
                Agent currentAgent = item->data(Qt::UserRole).value<Agent>();
                LOG_DEBUG("\n选中agent: \n - uuid: {}\n - name: {}\n - description: {}\n - children: {}\n\t- context: {}\n\t- systemPrompt: {}\n\t- modelName: {}\n\t- temperature: {}\n\t- topP: {}\n\t- maxTokens: {}\n\t- mcpServers_count: {} ",
                          currentAgent.uuid, currentAgent.name, currentAgent.description, currentAgent.children, currentAgent.context, currentAgent.systemPrompt, currentAgent.modelName, currentAgent.temperature, currentAgent.topP, currentAgent.maxTokens, currentAgent.mcpServers.count());
            });
    // m_listWidgetHistory
    m_listWidgetHistory = new QListWidget(this);
    connect(m_listWidgetHistory, &QListWidget::itemClicked, this,
            [](QListWidgetItem *item)
            {
                Conversation currentConversation = item->data(Qt::UserRole).value<Conversation>();
                LOG_DEBUG("\n选中对话: \n - uuid: {}\n - summary: {}\n\t - createdTime: {}\n\t - updatedTime: {}",
                          currentConversation.uuid, currentConversation.summary, currentConversation.createdTime.toString("yyyy-MM-dd HH:mm:ss"), currentConversation.updatedTime.toString("yyyy-MM-dd HH:mm:ss"));
            });
#ifdef QT_DEBUG
    for (int i = 0; i < 50; ++i)
    {
        // QString nameAgent = "agent实例测试" + QString::number(i + 1);
        // QListWidgetItem *itemAgent = new QListWidgetItem();
        // itemAgent->setText(nameAgent);
        // itemAgent->setData(Qt::UserRole, QVariant::fromValue(Agent(nameAgent, generateUuid(), QRandomGenerator::global()->bounded(101), QRandomGenerator::global()->bounded(11), generateUuid(), generateUuid(), QRandomGenerator::global()->bounded(101) / 10, QRandomGenerator::global()->bounded(101) / 10, QRandomGenerator::global()->bounded(10001))));
        // m_listWidgetAgent->addItem(itemAgent);

        QString nameConversation = "对话实例测试" + QString::number(i + 1);
        QListWidgetItem *itemConversation = new QListWidgetItem();
        itemConversation->setText(nameConversation);
        itemConversation->setData(Qt::UserRole, QVariant::fromValue(Conversation(generateUuid(), QDateTime::currentDateTime(), QDateTime::currentDateTime())));
        m_listWidgetHistory->addItem(itemConversation);
    }
#endif
    // m_tabWidgetSiderBar
    m_tabWidgetSiderBar = new QTabWidget(this);
    m_tabWidgetSiderBar->addTab(m_listWidgetAgent, "助手");
    m_tabWidgetSiderBar->addTab(m_listWidgetHistory, "话题");
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
        itemAgent->setData(Qt::UserRole, QVariant::fromValue(*(agent.get())));
        m_listWidgetAgent->addItem(itemAgent);
    }
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
                    LOG_DEBUG("发送消息: {}", userInput);
            });
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
