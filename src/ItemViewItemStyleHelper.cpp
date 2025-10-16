#include "ItemViewItemStyleHelper.h"
#include "ColorRepository.h"
#include <QDebug>

void ItemViewItemStyleHelper::drawItemViewItemShape(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget)
{
    // 添加有效性检查
    if (!option || !painter || !option->rect.isValid())
    {
        qCritical() << "Null pointer in drawItemViewItemShape!";
        return;
    }
    qDebug() << "Drawing item view item:"
             << "rect=" << option->rect
             << "valid=" << option->rect.isValid()
             << "widget=" << (widget ? widget->objectName() : "null");

    // 检查尺寸是否合理（防止溢出）
    if (option->rect.width() > 10000 || option->rect.height() > 10000 ||
        option->rect.width() <= 0 || option->rect.height() <= 0)
    {
        qWarning() << "Invalid rect in drawItemViewItemShape:" << option->rect;
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    setupPainterForShape(option, painter, widget);
    painter->drawRoundedRect(option->rect.adjusted(0, SPACING_TOP, 0, 0), RADIUS, RADIUS);
    painter->restore();
}

void ItemViewItemStyleHelper::drawText(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget)
{
    if (!option || !painter || !option->rect.isValid())
    {
        return;
    }

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
    painter->drawText(rectText, option->text);
    painter->restore();
}

void ItemViewItemStyleHelper::drawMarket(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget)
{
    if (!option || !painter || !option->rect.isValid())
    {
        return;
    }

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
    Q_UNUSED(widget)

    if (!option)
    {
        qWarning() << "ItemViewItemStyleHelper::sizeFromContents: null option";
        return QSize(100, 30); // 返回合理的默认值
    }

    // 验证字体有效性
    QFont font = option->font;
    if (font.pixelSize() <= 0 && font.pointSize() <= 0)
    {
        qWarning() << "ItemViewItemStyleHelper::sizeFromContents: invalid font";
        font = QFont(); // 使用默认字体
    }

    QFontMetrics fm(font);
    int textWidth = fm.horizontalAdvance(option->text);
    int widthTotal = MARK_OFFSET_X + MARK_WIDTH + SPACING_MARK_TO_TEXT +
                     textWidth + PADDING_HORIZONTAL;
    int finalWidth = qMax(widthTotal, contentsSize.width());
    int heightText = fm.height();
    int heightTotal = PADDING_VERTICAL + heightText + PADDING_VERTICAL;
    int finalHeight = qMax(SPACING_TOP + heightTotal, contentsSize.height());
    return QSize(finalWidth, finalHeight);
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