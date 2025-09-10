#include "CNavigationBar.h"
#include <QPainter>
#include <QSvgRenderer>
#include <QMouseEvent>
#include "Logger.hpp"
#include "BorderDebugger.hpp"
#include "global.h"

CNavigationContentWidget::CNavigationContentWidget(QWidget *parent)
    : QWidget(parent), m_indexHovered(-1), m_indexSelected(0)
{
    new BorderDebugger(this, this);
    setMouseTracking(true);
    m_marginLeft = 8;
    m_marginTop = 8;
    m_marginRight = 8;
    m_marginBottom = 8;
    m_spacingItem = 3;
    m_borderRadiusBackground = 5;
    m_spacingIcon2Background = 3;
    m_spacingBackground2Text = 3;
    m_sizeFont = 8;
    m_colorFont = QColor("#000000");
    m_colorBackground = QColor("#FFFFFF");
    m_colorIconBackgroundHovered = QColor("#E5F3FF");
    m_colorIconBackgroundSelected = QColor("#CCE8FF");
    m_colorBorderIconBackground = QColor("#99D1FF");
}

void CNavigationContentWidget::addItemSvg(const QString &text, const QString &filename)
{
    m_items.append({text, filename, true});
    updateSize();
}

void CNavigationContentWidget::addNonSelectableItemSvg(const QString &text, const QString &filename)
{
    m_items.append({text, filename, false});
    updateSize();
}

QSize CNavigationContentWidget::sizeHint() const
{
    int availableWidth = width();
    if (availableWidth <= 0 && parentWidget())
        availableWidth = parentWidget()->width();
    if (availableWidth <= 0)
        availableWidth = 50;

    int widthItem = qMax(availableWidth - m_marginLeft - m_marginRight, 20);
    QFontMetrics fm(font());
    int heightText = fm.height();

    int heightItem = widthItem + m_spacingBackground2Text + heightText;
    int totalHeight = m_marginTop + m_marginBottom;
    if (!m_items.isEmpty())
    {
        totalHeight += m_items.size() * heightItem;
        totalHeight += (m_items.size() - 1) * m_spacingItem;
    }

    return QSize(availableWidth, totalHeight);
}

void CNavigationContentWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    setMinimumHeight(sizeHint().height());
}

void CNavigationContentWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QFont font = getGlobalFont();
    font.setPointSize(m_sizeFont);
    painter.setFont(font);
    QFontMetrics fm(font);
    m_heightText = fm.height();
    m_widthItem = width() - m_marginLeft - m_marginRight;
    m_heightItem = m_widthItem + m_spacingBackground2Text + m_heightText;

    // 绘制背景
    painter.fillRect(rect(), m_colorBackground);
    painter.setPen(m_colorBackground.darker(120));
    painter.drawLine(width() - 1, 0, width() - 1, height());

    // 绘制item
    if (m_widthItem <= 0)
        return;
    for (int i = 0; i < m_items.size(); ++i)
    {
        QRect rectItem(m_marginLeft, m_marginTop + i * (m_heightItem + m_spacingItem), m_widthItem, m_heightItem);
        painter.setPen(Qt::NoPen);
        if (m_indexSelected == i)
            painter.setBrush(m_colorIconBackgroundSelected);
        else if (m_indexHovered == i)
            painter.setBrush(m_colorIconBackgroundHovered);
        else
            painter.setBrush(m_colorBackground);
        if (m_indexSelected == i && m_indexHovered == i)
            painter.setPen(m_colorBorderIconBackground);
        QRect rectBackground = rectItem.adjusted(0, 0, 0, -(m_spacingBackground2Text + m_heightText));
        painter.drawRoundedRect(rectBackground, m_borderRadiusBackground, m_borderRadiusBackground);

        // 绘制SVG图标
        QSvgRenderer renderer(m_items[i].filename);
        if (renderer.isValid())
        {
            QRect rectIcon = rectBackground.adjusted(m_spacingIcon2Background, m_spacingIcon2Background, -m_spacingIcon2Background, -m_spacingIcon2Background);
            renderer.render(&painter, rectIcon);
        }

        // 绘制文字
        QRect rectText = rectItem.adjusted(0, rectBackground.height(), 0, 0);
        painter.setPen(m_colorFont);
        painter.drawText(rectText, Qt::AlignCenter, m_items[i].text);
    }
    painter.end();
}

void CNavigationContentWidget::mousePressEvent(QMouseEvent *event)
{
    int index = itemAt(event->pos());
    if (index != -1 && index != m_indexSelected)
    {
        if (m_items[index].selectable)
            m_indexSelected = index;
        emit indexChanged(index, m_items[m_indexSelected].text);
        update();
    }
}

void CNavigationContentWidget::mouseMoveEvent(QMouseEvent *event)
{
    int index = itemAt(event->pos());
    if (index != m_indexHovered)
    {
        m_indexHovered = index;
        update();
    }
}

void CNavigationContentWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    if (m_indexHovered != -1)
    {
        m_indexHovered = -1;
        update();
    }
}

int CNavigationContentWidget::itemAt(const QPoint &pos) const
{
    if (m_widthItem <= 0)
        return -1;
    for (int i = 0; i < m_items.size(); ++i)
    {
        QRect rectCurrent(m_marginLeft, m_marginTop + i * (m_heightItem + m_spacingItem), m_widthItem, m_heightItem);
        if (rectCurrent.contains(pos))
        {
            return i;
        }
    }
    return -1;
}

void CNavigationContentWidget::updateSize()
{
    updateGeometry();
    update();
}

CNavigationBar::CNavigationBar(QWidget *parent)
    : QScrollArea(parent)
{
    initWidget();
}

CNavigationBar::~CNavigationBar()
{
}

void CNavigationBar::initWidget()
{
    m_contentWidget = new CNavigationContentWidget(this);
    setWidget(m_contentWidget);
    setWidgetResizable(true);
    setFrameShape(QFrame::NoFrame);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(m_contentWidget, &CNavigationContentWidget::indexChanged, this, &CNavigationBar::indexChanged);
}

void CNavigationBar::addItemSvg(const QString &text, const QString &filename)
{
    m_contentWidget->addItemSvg(text, filename);
}

void CNavigationBar::addNonSelectableItemSvg(const QString &text, const QString &filename)
{
    m_contentWidget->addNonSelectableItemSvg(text, filename);
}