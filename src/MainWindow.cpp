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
    // --- m_navigationBar ---
    m_navigationBar = new CNavigationBar(this);
    m_navigationBar->setMinimumWidth(30);
    m_navigationBar->setMaximumWidth(50);
    m_navigationBar->addItemSvg("测试01", "://image/agent.svg");
    m_navigationBar->addItemSvg("测试02", "://image/settings.svg");
    m_navigationBar->addItemSvg("测试03", "://image/agent.svg");
    m_navigationBar->addItemSvg("测试04", "://image/settings.svg");
    m_navigationBar->addItemSvg("测试05", "://image/agent.svg");
    m_navigationBar->addItemSvg("测试06", "://image/settings.svg");
    m_navigationBar->addItemSvg("测试07", "://image/agent.svg");
    m_navigationBar->addItemSvg("测试08", "://image/settings.svg");
    m_navigationBar->addItemSvg("测试09", "://image/agent.svg");
    m_navigationBar->addItemSvg("测试10", "://image/settings.svg");
    // --- m_splitter ---
    m_splitter = new QSplitter(this);
    m_splitter->setChildrenCollapsible(false);
    m_splitter->setContentsMargins(0, 0, 0, 0);
    m_splitter->addWidget(m_navigationBar);
    m_splitter->addWidget(new BaseWidget(this));
}

void MainWindow::initLayout()
{
    QHBoxLayout *h_layout = new QHBoxLayout(this);
    h_layout->setSpacing(0);
    h_layout->addWidget(m_splitter);
}
