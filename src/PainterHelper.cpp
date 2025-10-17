#include "PainterHelper.h"
#include "ColorRepository.h"

void PainterHelper::drawBackground(QPainter *painter, const QStyleOptionViewItem &option, QRect rectBackground, int radius, const QColor &colorNormalBackground)
{
    if (!painter)
        return;
    painter->save();
    painter->setPen(Qt::NoPen);
    // 选中状态
    if (option.state & QStyle::State_Selected)
    {
        // 选中且悬停
        if (option.state & QStyle::State_MouseOver)
        {
            QPen pen(ColorRepository::listSelectedAndHoveredOutlineColor());
            pen.setWidth(OUTLINE_WIDTH);
            painter->setPen(pen);
        }
        painter->setBrush(QBrush(ColorRepository::listSelectedBackgroundColor()));
        painter->drawRoundedRect(rectBackground, radius, radius);
    }
    // 悬停状态
    else if (option.state & QStyle::State_MouseOver)
    {
        painter->setBrush(QBrush(ColorRepository::listHoveredBackgroundColor()));
        painter->drawRoundedRect(rectBackground, radius, radius);
    }
    // 普通状态
    else
    {
        painter->setBrush(QBrush(colorNormalBackground.isValid() ? colorNormalBackground : ColorRepository::basicBackground()));
        painter->drawRoundedRect(rectBackground, radius, radius);
    }
    painter->restore();
}