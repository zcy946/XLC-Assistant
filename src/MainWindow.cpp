#include "MainWindow.h"
#include <QHBoxLayout>
#include "Logger.hpp"

MainWindow::MainWindow(QWidget *parent)
    : BaseWidget(parent)
{
    initUI();
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
    connect(m_navigationBar, &CNavigationBar::indexChanged, this, &MainWindow::on_navigationBar_indexChanged);
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

void MainWindow::on_navigationBar_indexChanged(int index, const QString &text)
{
    if (index < 0 || index > m_stackedLayout->count() - 1)
        return;
    LOG_DEBUG("导航至: {} - {}", index, text.toStdString());
    m_stackedLayout->setCurrentIndex(index);
}