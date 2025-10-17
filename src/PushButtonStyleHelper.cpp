#include "PushButtonStyleHelper.h"
#include "ColorRepository.h"
#include <QGraphicsDropShadowEffect>

void PushButtonStyleHelper::drawButtonShape(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    setupPainterForShape(option, painter, widget);
    painter->drawRoundedRect(QRectF(option->rect).adjusted(FRAME_WIDTH, FRAME_WIDTH, -(FRAME_WIDTH), -(FRAME_WIDTH)),
                             RADIUS,
                             RADIUS);
    painter->restore();
}

void PushButtonStyleHelper::drawText(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget)
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
        textColor = ColorRepository::text();
    }
    painter->setPen(QColor(textColor));
    QFont newFont = painter->font();
    if (widget)
    {
        newFont = widget->font();
    }
    newFont.setPixelSize(FONT_SIZE);
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
        shadowEffect->setBlurRadius(SHADOW_BLUR_RADIUS);
        shadowEffect->setColor(ColorRepository::shadowColor());
        shadowEffect->setOffset(SHADOW_OFFSET_X, SHADOW_OFFSET_Y);
        button->setGraphicsEffect(shadowEffect);
    }
}

QSize PushButtonStyleHelper::sizeFromContents(const QStyleOptionButton *option, QSize contentsSize, const QWidget *widget) const
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    return QSize(qMax(MIN_WIDTH, contentsSize.width() + PADDING_HORIZONTAL * 2 + FRAME_WIDTH * 2),
                 contentsSize.height() + PADDING_VERTICAL * 2 + FRAME_WIDTH * 2);
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
        painter->setPen(ColorRepository::buttonOutlineColor());
        painter->setBrush(ColorRepository::buttonBackground());
    }
    // 按下
    else if (option->state & QStyle::State_Sunken)
    {
        painter->setPen(ColorRepository::buttonPressedOutlineColor());
        painter->setBrush(ColorRepository::buttonPressedBackground());
    }
    // 悬停
    else if (option->state & QStyle::State_MouseOver)
    {
        painter->setPen(QPen(ColorRepository::buttonHoverOutlineColor(), FRAME_WIDTH));
        painter->setBrush(ColorRepository::buttonHoveredBackground());
    }
    // 默认
    else
    {
        painter->setPen(ColorRepository::buttonOutlineColor());
        painter->setBrush(ColorRepository::buttonBackground());
    }
}