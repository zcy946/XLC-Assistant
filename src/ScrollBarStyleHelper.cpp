#include "ScrollBarStyleHelper.h"
#include <QProxyStyle>
#include <QPainterPath>
#include "ColorRepository.h"
#include <QtMath>
#include <QScrollBar>

void ScrollBarStyleHelper::drawBackground(const QStyleOptionSlider *option, QPainter *painter, const QWidget *widget)
{
    if (!qobject_cast<const QScrollBar *>(widget))
    {
        return;
    }
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);
    painter->setBrush(ColorRepository::scrollBarBackgroundColor());
    // painter->drawRoundedRect(option->rect, RADIUS, RADIUS);
    painter->drawRect(option->rect);
    painter->restore();
}

void ScrollBarStyleHelper::drawGroove(const QStyleOptionSlider *option, QPainter *painter, const QWidget *widget, const QRect &grooveRect)
{
    if (!qobject_cast<const QScrollBar *>(widget))
    {
        return;
    }
    if (option->subControls & QStyle::SC_ScrollBarGroove)
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setBrush(ColorRepository::scrollBarBackgroundColor());
        painter->setPen(Qt::NoPen);
        painter->drawRect(grooveRect);
        painter->restore();
    }
}

void ScrollBarStyleHelper::drawSlider(const QStyleOptionSlider *option, QPainter *painter, const QWidget *widget, const QRect &rectSlider)
{
    if (!qobject_cast<const QScrollBar *>(widget))
    {
        return;
    }
    if (option->subControls & QStyle::SC_ScrollBarSlider)
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        QRect rectDrawSlider;
        if (option->orientation == Qt::Horizontal)
        {
            rectDrawSlider = rectSlider.adjusted(SPACING_SLIDER_TO_ARROW, 0, -SPACING_SLIDER_TO_ARROW, 0);
        }
        else
        {
            rectDrawSlider = rectSlider.adjusted(0, SPACING_SLIDER_TO_ARROW, 0, -SPACING_SLIDER_TO_ARROW);
        }
        QColor sliderColor = ColorRepository::scrollBarSliderColor();
        if (option->state & QStyle::State_MouseOver)
            sliderColor = ColorRepository::scrollBarSliderHoveredColor();
        if (option->state & QStyle::State_Sunken)
            sliderColor = ColorRepository::scrollBarSliderSelectedColor();
        painter->setBrush(sliderColor);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(rectDrawSlider, SCROLLBAR_EXTENT / 2, SCROLLBAR_EXTENT / 2);
        painter->restore();
    }
}

void ScrollBarStyleHelper::drawSubControls(const QStyleOptionSlider *option, QPainter *painter, const QWidget *widget, const QRect &rectSubLine, const QRect &rectAddLine)
{
    if (!qobject_cast<const QScrollBar *>(widget))
    {
        return;
    }
    // 只绘制 Line 按钮的箭头，跳过 Page 按钮 (滚动条两端的按钮)
    if (!(option->subControls & QStyle::SC_ScrollBarSubLine) && !(option->subControls & QStyle::SC_ScrollBarAddLine))
        return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setBrush(ColorRepository::scrollBarArrowColor());
    painter->setPen(Qt::NoPen);
    if (option->orientation == Qt::Horizontal)
    {
        // 左三角(行减按钮)
        qreal centerLeftX = rectSubLine.x() + rectSubLine.width() / 2;
        qreal centerRightX = rectAddLine.x() + rectAddLine.width() / 2;
        qreal centerY = rectSubLine.height() / 2;
        QPainterPath leftPath;
        leftPath.moveTo(centerLeftX - qCos(30 * PI / 180.0) * SIDE_LENGTH / 2, centerY);
        leftPath.lineTo(centerLeftX + qCos(30 * PI / 180.0) * SIDE_LENGTH / 2, centerY + SIDE_LENGTH / 2);
        leftPath.lineTo(centerLeftX + qCos(30 * PI / 180.0) * SIDE_LENGTH / 2, centerY - SIDE_LENGTH / 2);
        leftPath.closeSubpath();
        painter->drawPath(leftPath);

        // 右三角(行增按钮)
        QPainterPath rightPath;
        rightPath.moveTo(centerRightX + qCos(30 * PI / 180.0) * SIDE_LENGTH / 2, centerY);
        rightPath.lineTo(centerRightX - qCos(30 * PI / 180.0) * SIDE_LENGTH / 2, centerY + SIDE_LENGTH / 2);
        rightPath.lineTo(centerRightX - qCos(30 * PI / 180.0) * SIDE_LENGTH / 2, centerY - SIDE_LENGTH / 2);
        rightPath.closeSubpath();
        painter->drawPath(rightPath);
    }
    else
    {
        qreal centerToTop = (SIDE_LENGTH / 2) / qCos(30 * M_PI / 180.0);
        qreal centerToBottom = (SIDE_LENGTH / 2) * qTan(30 * M_PI / 180.0);
        // 上三角(行增按钮)
        qreal centerX = rectSubLine.width() / 2.0;
        qreal centerUpY = rectSubLine.center().y() + 2;
        qreal centerDownY = rectAddLine.center().y() + 2;
        QPainterPath upPath;
        upPath.moveTo(centerX, centerUpY - centerToTop);
        upPath.lineTo(centerX + SIDE_LENGTH / 2, centerUpY + centerToBottom);
        upPath.lineTo(centerX - SIDE_LENGTH / 2, centerUpY + centerToBottom);
        upPath.closeSubpath();
        painter->drawPath(upPath);

        // 下三角(行减按钮)
        QPainterPath downPath;
        downPath.moveTo(centerX, centerDownY + centerToBottom);
        downPath.lineTo(centerX + SIDE_LENGTH / 2, centerDownY - centerToTop);
        downPath.lineTo(centerX - SIDE_LENGTH / 2, centerDownY - centerToTop);
        downPath.closeSubpath();
        painter->drawPath(downPath);
    }
    painter->restore();
}

int ScrollBarStyleHelper::ScrollBarStyleHelper::scrollBarExtent() const
{
    return SCROLLBAR_EXTENT;
}