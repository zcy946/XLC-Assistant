#include "MainWindow.h"
#include <QHBoxLayout>
#include "Logger.hpp"
#include "DataManager.h"
#include "EventBus.h"
#include "ToastManager.h"
#include <QSplitter>

MainWindow::MainWindow(QWidget *parent)
    : BaseWidget(parent)
{
    // 加载数据
    DataManager::getInstance()->init();
    initUI();
    connect(EventBus::getInstance().get(), &EventBus::sig_pageSwitched, this, &MainWindow::handlePageSwitched);
}

MainWindow::~MainWindow()
{
}

void MainWindow::initWidget()
{
    ToastManager::getInstance()->init(this);
}

void MainWindow::initItems()
{
    // m_pageChat
    m_pageChat = new PageChat(this);
    m_pageChat->setObjectName("page_聊天");
    m_pages.insert(m_pageChat->objectName(), m_pageChat);
    // m_pageSettings
    m_pageSettings = new PageSettings(this);
    m_pageSettings->setObjectName("page_设置");
    m_pages.insert(m_pageSettings->objectName(), m_pageSettings);
    // m_navigationBar
    m_navigationBar = new XlcNavigationBar(this);
    // m_navigationBar->setFixedWidth(300);
    m_navigationBar->addNavigationNode("聊天", QChar(0xefa0), m_pageChat->objectName());
    m_navigationBar->addNavigationNode("设置", QChar(0xedab), m_pageSettings->objectName());
    m_navigationBar->setSelectedItem("聊天");
    connect(m_navigationBar, &XlcNavigationBar::sig_currentItemChanged, this, &MainWindow::handleNavigationBarItemChanged);
#ifdef QT_DEBUG
    // 添加测试项目
    for (int i = 0; i < 10; ++i)
    {
        m_navigationBar->addNavigationNode("Item" + QString::number(i), QChar(0xe9ac));
    }
#endif
}

void MainWindow::initLayout()
{
    // m_stackedLayout
    QWidget *widgetPages = new QWidget(this);
    m_stackedLayout = new QStackedLayout(widgetPages);
    m_stackedLayout->setContentsMargins(0, 0, 0, 0);
    m_stackedLayout->setSpacing(0);
    m_stackedLayout->addWidget(m_pageChat);
    m_stackedLayout->addWidget(m_pageSettings);
    // splitter
    QSplitter *splitter = new QSplitter(this);
    splitter->addWidget(m_navigationBar);
    splitter->addWidget(widgetPages);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 7);
    // vLayout
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addWidget(splitter);

    // // h_layout
    // QHBoxLayout *h_layout = new QHBoxLayout(this);
    // h_layout->setSpacing(0);
    // h_layout->setContentsMargins(0, 0, 0, 0);
    // h_layout->addWidget(m_navigationBar);
    // // m_stackedLayout
    // m_stackedLayout = new QStackedLayout();
    // m_stackedLayout->setContentsMargins(0, 0, 0, 0);
    // m_stackedLayout->setSpacing(0);
    // m_stackedLayout->addWidget(m_pageChat);
    // m_stackedLayout->addWidget(m_pageSettings);
    // h_layout->addLayout(m_stackedLayout);
}

void MainWindow::handleNavigationBarItemChanged(const QString &targetId)
{
    if (!m_pages.contains(targetId))
        return;
    m_stackedLayout->setCurrentWidget(m_pages.value(targetId));
    XLC_LOG_TRACE("Navigating to target page (targetId={})", targetId);
    // WARNING：必须重新置顶ToastManager，否则切换stackedLayout会被遮盖
    ToastManager::getInstance()->raise();
}

void MainWindow::handlePageSwitched(const QVariant &data)
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
            if (!agentUuid.isEmpty() && !conversationUuid.isEmpty())
            {
                m_navigationBar->setSelectedItem("聊天");
            }
            else
            {
                XLC_LOG_WARN("Failed to switch, agent or conversation UUID is empty.");
            }
            break;
        }
        default:
        {
            XLC_LOG_WARN("Page not found (id={})", id);
            break;
        }
        }
    }
    else
    {
        XLC_LOG_WARN("Failed to process page switch event (typename={}): unexpected data type", data.typeName());
    }
}