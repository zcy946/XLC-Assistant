#include "ItemViewItemStyleHelper.h"
#include "ColorRepository.h"

void ItemViewItemStyleHelper::drawItemViewItemShape(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    setupPainterForShape(option, painter, widget);
    painter->drawRoundedRect(option->rect.adjusted(0, SPACING_TOP, 0, -SPACING_TOP), RADIUS, RADIUS);
    painter->restore();
}

void ItemViewItemStyleHelper::drawText(const QStyleOptionViewItem *option, QPainter *painter, QRect rectTextOriginal)
{
    if (option->text.isEmpty())
        return;
    painter->save();
    QColor textColor;
    if (option->state & QStyle::State_Sunken)
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
    QFontMetrics fm(option->font);
    QRectF rectText = rectTextOriginal.adjusted(MARK_OFFSET_X + MARK_WIDTH + SPACING_MARK_TO_TEXT, SPACING_TOP + PADDING_VERTICAL, 0, -SPACING_TOP - PADDING_VERTICAL);
    painter->drawText(rectText, option->text);
    painter->restore();
}

void ItemViewItemStyleHelper::drawMarket(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget)
{
    if (option->state & QStyle::State_Selected)
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(Qt::NoPen);
        painter->setBrush(ColorRepository::primaryNormal());
        painter->drawRoundedRect(QRectF(option->rect.x() + MARK_OFFSET_X,
                                        option->rect.y() + SPACING_TOP + MARK_PADDING_TOP,
                                        option->rect.x() + MARK_WIDTH,
                                        option->rect.height() - SPACING_TOP - MARK_PADDING_TOP - MARK_PADDING_BOTTOM),
                                 MARK_RADIUS,
                                 MARK_RADIUS);
        painter->restore();
    }
}

QSize ItemViewItemStyleHelper::sizeFromContents(const QStyleOptionViewItem *option, QSize sizeOriginal, const QWidget *widget) const
{
    Q_UNUSED(widget)
    int width = MARK_OFFSET_X + MARK_WIDTH + SPACING_MARK_TO_TEXT + sizeOriginal.width();
    int height = SPACING_TOP + + PADDING_VERTICAL + sizeOriginal.height() + PADDING_VERTICAL;
    return QSize(width, height);
}

void ItemViewItemStyleHelper::setupPainterForShape(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget)
{
    Q_UNUSED(widget)
    // 选中
    if (option->state & QStyle::State_Selected)
    {
        // 选中且悬停
        if (option->state & QStyle::State_MouseOver)
        {
            QPen pen(ColorRepository::listSelectedAndHoveredOutlineColor());
            painter->setPen(Qt::NoPen);
            painter->setBrush(ColorRepository::itemViewItemSelectedAndHoveredBackgroundColor());
        }
        else
        {
            painter->setPen(Qt::NoPen);
            painter->setBrush(ColorRepository::itemViewItemSelectedBackgroundColor());
        }
    }
    // 悬停
    else if (option->state & QStyle::State_MouseOver)
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(ColorRepository::itemViewItemHoveredBackgroundColor());
    }
    // 默认
    else
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(ColorRepository::itemViewItemBackgroundColor());
    }
}