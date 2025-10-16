#include "ItemViewItemStyleHelper.h"
#include "ColorRepository.h"
#include <QDebug>

void ItemViewItemStyleHelper::drawItemViewItemShape(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    setupPainterForShape(option, painter, widget);
    painter->drawRoundedRect(option->rect.adjusted(0, SPACING_TOP, 0, 0), RADIUS, RADIUS);
    painter->restore();
}

void ItemViewItemStyleHelper::drawText(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget)
{
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
    QRectF rectText = option->rect.adjusted(MARK_OFFSET_X + MARK_WIDTH + SPACING_MARK_TO_TEXT,
                                            PADDING_VERTICAL + SPACING_TOP,
                                            -PADDING_HORIZONTAL,
                                            -PADDING_VERTICAL);
    QString elidedText = fm.elidedText(option->text, Qt::ElideRight, rectText.width());
    painter->drawText(rectText, elidedText);
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

QSize ItemViewItemStyleHelper::sizeFromContents(const QStyleOptionViewItem *option, QSize contentsSize, const QWidget *widget) const
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    QFontMetrics fm(option->font);
    // NOTE 这种方法返回的宽度会使 `ScrollBarAsNeeded` 永久失效
    QString ellipsis = QChar(0x2026); // ( Unicode 省略号)qt用于表示省略字符的代码
    int minContentTextWidth = fm.horizontalAdvance(ellipsis);
    int widthTotal = MARK_OFFSET_X + MARK_WIDTH + SPACING_MARK_TO_TEXT + minContentTextWidth + PADDING_HORIZONTAL;
    // int widthTotal = MARK_OFFSET_X + MARK_WIDTH + SPACING_MARK_TO_TEXT + fm.horizontalAdvance(option->text) + PADDING_HORIZONTAL; // 这种方法没问题
    int heightText = fm.height();
    int heightTotal = PADDING_VERTICAL + heightText + PADDING_VERTICAL;
    return QSize(widthTotal, SPACING_TOP + heightTotal);
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