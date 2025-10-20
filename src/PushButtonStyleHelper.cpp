#include "PushButtonStyleHelper.h"
#include "ColorRepository.h"
#include <QGraphicsDropShadowEffect>
#include <QPushButton>
#include <QPainterPath>

void PushButtonStyleHelper::drawBackground(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    setupPainterForShape(option, painter, widget);
    painter->drawRoundedRect(QRectF(option->rect).adjusted(RADIUS_SHADOW + WIDTH_BORDER, RADIUS_SHADOW + WIDTH_BORDER, -(RADIUS_SHADOW + WIDTH_BORDER), -(RADIUS_SHADOW + WIDTH_BORDER)),
                             RADIUS,
                             RADIUS);
    painter->restore();
}

void PushButtonStyleHelper::drawHemline(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget) const
{
    if (option->state & QStyle::State_Sunken)
        return;
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QColor color = ColorRepository::buttonHemlineColor();
    color.setAlpha(7);
    painter->setPen(color);
    QRect rectForeground = option->rect.adjusted(WIDTH_BORDER, WIDTH_BORDER, -WIDTH_BORDER, -WIDTH_BORDER);
    painter->drawLine(rectForeground.x() + RADIUS,
                      rectForeground.height() - RADIUS_SHADOW,
                      rectForeground.width() - RADIUS,
                      rectForeground.height() - RADIUS_SHADOW);
    painter->restore();
}

void PushButtonStyleHelper::drawText(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget) const
{
    painter->save();
    QColor textColor;
    if (!(option->state & QStyle::State_Enabled))
    {
        textColor = ColorRepository::disabledTextColor();
    }
    else if (option->state & QStyle::State_Sunken)
    {
        textColor = ColorRepository::pressedTextColor();
    }
    else if (option->state & QStyle::State_MouseOver)
    {
        textColor = ColorRepository::hoverTextColor();
    }
    else
    {
        textColor = ColorRepository::basicText();
    }
    painter->setPen(QColor(textColor));
    QFont newFont = painter->font();
    if (widget)
    {
        newFont = widget->font();
    }
    newFont.setPixelSize(SIZE_FONT);
    painter->setFont(newFont);

    painter->drawText(option->rect, Qt::AlignCenter, option->text);
    painter->restore();
}

void PushButtonStyleHelper::drawShadow(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget) const
{
    Q_UNUSED(widget)
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    QColor color = ColorRepository::shadowColor();
    for (int i = 0; i < RADIUS_SHADOW; i++)
    {
        path.addRoundedRect(option->rect.x() + RADIUS_SHADOW - i,
                            option->rect.y() + RADIUS_SHADOW - i,
                            option->rect.width() - (RADIUS_SHADOW - i) * 2,
                            option->rect.height() - (RADIUS_SHADOW - i) * 2,
                            RADIUS + i,
                            RADIUS + i);
        int alpha = 1 * (RADIUS_SHADOW - i + 1);
        color.setAlpha(alpha > 255 ? 255 : alpha);
        painter->setPen(color);
        painter->drawPath(path);
    }
    painter->restore();
}

QSize PushButtonStyleHelper::sizeFromContents(const QStyleOptionButton *option, QSize contentsSize, const QWidget *widget) const
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    return QSize(qMax(WIDTH_MIN, contentsSize.width() + PADDING_HORIZONTAL * 2 + WIDTH_BORDER * 2 + RADIUS_SHADOW * 2),
                 contentsSize.height() + PADDING_VERTICAL * 2 + WIDTH_BORDER * 2 + RADIUS_SHADOW * 2);
}

int PushButtonStyleHelper::padding()
{
    return qMax(PADDING_HORIZONTAL, PADDING_VERTICAL);
}

void PushButtonStyleHelper::setupPainterForShape(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget) const
{
    Q_UNUSED(widget)
    // 禁用
    if (!(option->state & QStyle::State_Enabled))
    {
        painter->setPen(ColorRepository::buttonBorderColor());
        painter->setBrush(ColorRepository::buttonBackgroundColor());
    }
    // 按下
    else if (option->state & QStyle::State_Sunken)
    {
        painter->setPen(ColorRepository::buttonPressedBorderColor());
        painter->setBrush(ColorRepository::buttonPressedBackgroundColor());
    }
    // 悬停
    else if (option->state & QStyle::State_MouseOver)
    {
        painter->setPen(QPen(ColorRepository::buttonHoverOutlineColor(), WIDTH_BORDER));
        painter->setBrush(ColorRepository::buttonHoveredBackgroundColor());
    }
    // 默认
    else
    {
        painter->setPen(ColorRepository::buttonBorderColor());
        painter->setBrush(ColorRepository::buttonBackgroundColor());
    }
}