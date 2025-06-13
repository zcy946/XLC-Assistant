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
    m_navigationBar->setFixedWidth(50);
    connect(m_navigationBar, &CNavigationBar::indexChanged, this,
            [](int index, const QString &text)
            {
                LOG_DEBUG("current index: {}-{}", index, text.toStdString());
            });

    // 添加测试项目
    m_navigationBar->addItemSvg("聊天", "://image/message-3-line.svg");
    m_navigationBar->addItemSvg("设置", "://image/settings-3-line.svg");
    for (int i = 0; i < 20; ++i)
        m_navigationBar->addItemSvg("测试" + QString::number(i + 1), "://image/fire-line.svg");
}

void MainWindow::initLayout()
{
    QHBoxLayout *h_layout = new QHBoxLayout(this);
    h_layout->setSpacing(0);
    h_layout->setContentsMargins(0, 0, 0, 0);
    h_layout->addWidget(m_navigationBar);
    h_layout->addWidget(new BaseWidget(this));
}