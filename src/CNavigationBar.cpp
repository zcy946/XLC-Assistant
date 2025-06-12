#include "CNavigationBar.h"
#include <QSvgRenderer>
#include "Logger.hpp"

CNavigationBar::CNavigationBar(QWidget *parent)
    : BaseWidget(parent)
{
}

CNavigationBar::~CNavigationBar()
{
}

void CNavigationBar::initWidget()
{
}

void CNavigationBar::initItems()
{
}

void CNavigationBar::initLayout()
{
}

void CNavigationBar::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    int sizeIcon = width() - m_marginLeft - m_marginRight;
    for (int i = 0; i < m_items.size(); ++i)
    {
        QRect rectItem(m_marginLeft, m_marginTop + i * (sizeIcon + m_spacing), sizeIcon, sizeIcon);
        painter.fillRect(rectItem, QColor("#1F1F1F"));
    }
    painter.end();
}

void CNavigationBar::addItemSvg(const QString &text, const QString &filename)
{
    m_items.append(item({text, filename, true}));
}

void CNavigationBar::addNonSelectableItemSvg(const QString &text, const QString &filename)
{
    m_items.append(item({text, filename, false}));
}
