#include "PushButtonStyleHelper.h"
#include "ColorRepository.h"
#include <QGraphicsDropShadowEffect>
#include <QPushButton>

void PushButtonStyleHelper::drawButtonShape(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget)
{
    // 检查请求是否由 QPushButton 发起
    if (!qobject_cast<const QPushButton *>(widget))
    {
        return;
    }
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    setupPainterForShape(option, painter, widget);
    painter->drawRoundedRect(QRectF(option->rect).adjusted(WIDTH_BORDER, WIDTH_BORDER, -(WIDTH_BORDER), -(WIDTH_BORDER)),
                             RADIUS,
                             RADIUS);
    painter->restore();
}

void PushButtonStyleHelper::drawText(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget)
{
    // 检查请求是否由 QPushButton 发起
    if (!qobject_cast<const QPushButton *>(widget))
    {
        return;
    }
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

void PushButtonStyleHelper::drawShadow(QPushButton *button)
{
    // 确保只有一个阴影效果
    if (!button->graphicsEffect())
    {
        QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(button);
        shadowEffect->setBlurRadius(RADIUS_SHADOW);
        shadowEffect->setColor(ColorRepository::shadowColor());
        shadowEffect->setOffset(OFFSET_X_SHADOW, OFFSET_Y_SHADOW);
        button->setGraphicsEffect(shadowEffect);
    }
}

QSize PushButtonStyleHelper::sizeFromContents(const QStyleOptionButton *option, QSize contentsSize, const QWidget *widget) const
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    return QSize(qMax(WIDTH_MIN, contentsSize.width() + PADDING_HORIZONTAL * 2 + WIDTH_BORDER * 2),
                 contentsSize.height() + PADDING_VERTICAL * 2 + WIDTH_BORDER * 2);
}

int PushButtonStyleHelper::padding()
{
    return qMax(PADDING_HORIZONTAL, PADDING_VERTICAL);
}

void PushButtonStyleHelper::setupPainterForShape(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget)
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