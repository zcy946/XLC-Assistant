#include "MainWindow.h"
#include <QHBoxLayout>
#include <QDebug>

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
    m_navigationBar->addItemSvg("测试01", "://image/agent.svg");
    m_navigationBar->addItemSvg("测试02", "://image/settings.svg");
}

void MainWindow::initLayout()
{
    QHBoxLayout *h_layout = new QHBoxLayout(this);
    h_layout->addWidget(m_navigationBar);
}
