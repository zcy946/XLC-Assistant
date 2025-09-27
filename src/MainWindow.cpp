#include "MainWindow.h"
#include <QHBoxLayout>
#include "Logger.hpp"
#include "DataManager.h"
#include "EventBus.h"

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
}

void MainWindow::initItems()
{
    // m_navigationBar
    m_navigationBar = new CNavigationBar(this);
    m_navigationBar->setFixedWidth(60);
    m_navigationBar->addItemSvg("聊天", "://image/message-3-line.svg");
    m_navigationBar->addItemSvg("设置", "://image/settings-3-line.svg");
    connect(m_navigationBar, &CNavigationBar::indexChanged, this, &MainWindow::handleNavigationBarIndexChanged);
#ifdef QT_DEBUG
    // 添加测试项目
    for (int i = 0; i < 20; i += 2)
    {
        m_navigationBar->addItemSvg("Item" + QString::number(i + 1), "://image/fire-line.svg");
        m_navigationBar->addNonSelectableItemSvg("NonSelectableItem" + QString::number(i + 2), "://image/fire-line.svg");
    }
#endif

    // m_pageChat
    m_pageChat = new PageChat(this);
    // m_pageSettings
    m_pageSettings = new PageSettings(this);
}

void MainWindow::initLayout()
{
    // h_layout
    QHBoxLayout *h_layout = new QHBoxLayout(this);
    h_layout->setSpacing(0);
    h_layout->setContentsMargins(0, 0, 0, 0);
    h_layout->addWidget(m_navigationBar);
    // m_stackedLayout
    m_stackedLayout = new QStackedLayout();
    m_stackedLayout->setContentsMargins(0, 0, 0, 0);
    m_stackedLayout->setSpacing(0);
    m_stackedLayout->addWidget(m_pageChat);
    m_stackedLayout->addWidget(m_pageSettings);
    h_layout->addLayout(m_stackedLayout);
}

void MainWindow::handleNavigationBarIndexChanged(int index, const QString &text)
{
    if (index < 0 || index > m_stackedLayout->count() - 1)
        return;
    XLC_LOG_TRACE("Navigating to index (index={}, text={})", index, text);
    m_stackedLayout->setCurrentIndex(index);
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
                m_navigationBar->setCurrentText("聊天");
            }
            else
            {
                XLC_LOG_ERROR("Failed to switch, agent or conversation UUID is empty.");
            }
            break;
        }
        default:
        {
            XLC_LOG_ERROR("Page not found (id={})", id);
            break;
        }
        }
    }
    else
    {
        XLC_LOG_ERROR("Failed to process page switch event (typename={}): unexpected data type", data.typeName());
    }
}