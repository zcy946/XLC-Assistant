#include "PushButtonStyleHelper.h"
#include "ColorRepository.h"
#include <QGraphicsDropShadowEffect>
#include <QDebug>

void PushButtonStyleHelper::drawButtonShape(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget)
{
    if (!option || !painter || !option->rect.isValid())
    {
        qCritical() << "Null pointer in drawButtonShape!";
        return;
    }
    qDebug() << "Drawing item view item:"
             << "rect=" << option->rect
             << "valid=" << option->rect.isValid()
             << "widget=" << (widget ? widget->objectName() : "null");

    if (option->rect.width() > 10000 || option->rect.height() > 10000 ||
        option->rect.width() <= 0 || option->rect.height() <= 0)
    {
        qWarning() << "Invalid rect in drawButtonShape:" << option->rect;
        return;
    }

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
    if (!option || !painter || !option->rect.isValid())
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
        textColor = ColorRepository::text();
    }
    painter->setPen(QColor(textColor));
    QFont newFont = painter->font();
    if (widget)
    {
        newFont = widget->font();
    }
    newFont.setPixelSize(14);
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
    // return QSize(qMax(MIN_WIDTH, contentsSize.width() + PADDING_HORIZONTAL * 2 + FRAME_WIDTH * 2), contentsSize.height() + PADDING_VERTICAL * 2 + FRAME_WIDTH * 2);
    // 验证输入尺寸
    if (!contentsSize.isValid() ||
        contentsSize.width() < 0 || contentsSize.width() > 10000 ||
        contentsSize.height() < 0 || contentsSize.height() > 10000)
    {
        qWarning() << "PushButtonStyleHelper::sizeFromContents: invalid contentsSize:" << contentsSize;
        contentsSize = QSize(80, 24); // 使用合理默认值
    }

    int width = qMax(MIN_WIDTH, contentsSize.width() + PADDING_HORIZONTAL * 2 + FRAME_WIDTH * 2);
    int height = contentsSize.height() + PADDING_VERTICAL * 2 + FRAME_WIDTH * 2;

    // 限制最终尺寸
    width = qBound(MIN_WIDTH, width, 10000);
    height = qBound(20, height, 10000);

    return QSize(width, height);
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