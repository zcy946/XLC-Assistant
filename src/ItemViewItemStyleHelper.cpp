#include "ItemViewItemStyleHelper.h"
#include "ColorRepository.h"
#include <QListView>
#include <QProxyStyle>
#include "XlcStyle.h"

void ItemViewItemStyleHelper::drawBackground(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    setupPainterForShape(option, painter, widget);
    painter->drawRoundedRect(option->rect.adjusted(0, SPACING_TOP, 0, -SPACING_TOP), RADIUS, RADIUS);
    painter->restore();
}

void ItemViewItemStyleHelper::drawText(const XlcStyle *style, const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget) const
{
    if (option->text.isEmpty())
        return;
    QRect rectText = style->subElementRect(QStyle::SE_ItemViewItemText, option, widget);
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
        textColor = ColorRepository::basicTextColor();
    }
    painter->setPen(QColor(textColor));
    painter->drawText(rectText, option->text);
    painter->restore();
}

void ItemViewItemStyleHelper::drawMarket(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget) const
{
    Q_UNUSED(widget)
    if (option->state & QStyle::State_Selected)
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(Qt::NoPen);
        painter->setBrush(ColorRepository::primaryNormal());
        painter->drawRoundedRect(QRectF(option->rect.x() + OFFSET_MARK_X,
                                        option->rect.y() + SPACING_TOP + PADDING_MARK_TOP,
                                        WIDTH_MARK,
                                        option->rect.height() - SPACING_TOP - PADDING_MARK_TOP - PADDING_MARK_BOTTOM),
                                 RADIUS_MARK,
                                 RADIUS_MARK);
        painter->restore();
    }
}

void ItemViewItemStyleHelper::drawCheckIndicator(const XlcStyle *style, const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget) const
{
    if (!(option->features & QStyleOptionViewItem::HasCheckIndicator))
    {
        return;
    }
    // 创建并填充一个用于绘制 CheckBox 的 QStyleOptionButton
    QStyleOptionButton optionCheckBox;
    optionCheckBox.rect = style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, option, widget);
    // 继承 item 的基本状态 (enabled/disabled)
    optionCheckBox.state = option->state & (QStyle::State_Enabled | QStyle::State_MouseOver);
    switch (option->checkState)
    {
    case Qt::Checked:
        optionCheckBox.state |= QStyle::State_On;
        break;
    case Qt::PartiallyChecked:
        optionCheckBox.state |= QStyle::State_NoChange;
        break;
    case Qt::Unchecked:
        optionCheckBox.state |= QStyle::State_Off;
        break;
    }
    style->drawPrimitive(QStyle::PE_IndicatorCheckBox, &optionCheckBox, painter, widget);
}

void ItemViewItemStyleHelper::drawBranchIndicator(const XlcStyle *style, const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget) const
{
    // 展开图标绘制
    if (option->state & QStyle::State_Children)
    {
        painter->save();
        QRect rectBranchIndicator = style->subElementRect(QStyle::SE_TreeViewDisclosureItem, option, widget);
        QFont iconFont = QFont(ICONFONT_NAME);
        painter->setFont(iconFont);
        painter->setPen(ColorRepository::basicTextColor());
        QChar iconfontBranchIndicator = (option->state & QStyle::State_Open) ? ICONFONT_AngleDown : ICONFONT_AngleRight;
        painter->drawText(rectBranchIndicator, Qt::AlignVCenter | Qt::AlignRight, iconfontBranchIndicator);
        painter->restore();
    }
}

QSize ItemViewItemStyleHelper::sizeFromContents(const QStyleOptionViewItem *option, QSize sizeOriginal, const QWidget *widget) const
{
    Q_UNUSED(widget)
    int width = OFFSET_MARK_X + WIDTH_MARK + SPACING_MARK_TO_TEXT + sizeOriginal.width();
    int height = SPACING_TOP + PADDING_VERTICAL + sizeOriginal.height() + PADDING_VERTICAL;
    return QSize(width, height);
}

QRect ItemViewItemStyleHelper::rectText(const QStyleOptionViewItem *option, const QWidget *widget, QRect rectTextOriginal) const
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    return rectTextOriginal.adjusted(OFFSET_MARK_X + WIDTH_MARK + SPACING_MARK_TO_TEXT,
                                     SPACING_TOP + PADDING_VERTICAL,
                                     0,
                                     -SPACING_TOP - PADDING_VERTICAL);
}

QRect ItemViewItemStyleHelper::rectCheckIndicator(const QStyleOptionViewItem *option, const QWidget *widget) const
{
    Q_UNUSED(widget)
    QRect rectCheckIndiactor = QRect(option->rect.x() + OFFSET_MARK_X + WIDTH_MARK + SPACING_MARK_TO_TEXT,
                                     option->rect.y() + (option->rect.height() - HEIGHT_CHECK_INDICATOR) / 2,
                                     WIDTH_CHECK_INDICATOR,
                                     HEIGHT_CHECK_INDICATOR);
    return rectCheckIndiactor;
}

QRect ItemViewItemStyleHelper::rectClickCheckIndicator(const XlcStyle *style, const QStyleOptionViewItem *option, const QWidget *widget)
{
    return style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, option, widget);
}

QRect ItemViewItemStyleHelper::rectBranchIndicator(const QStyleOptionViewItem *option, const QWidget *widget, QRect rectBranchIndicatorOriginal)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    return rectBranchIndicatorOriginal;
}

void ItemViewItemStyleHelper::setupPainterForShape(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget)
{
    Q_UNUSED(widget)
    painter->setPen(Qt::NoPen);
    // 选中
    if (option->state & QStyle::State_Selected)
    {
        // 选中且悬停
        if (option->state & QStyle::State_MouseOver)
        {
            painter->setBrush(ColorRepository::itemViewItemSelectedAndHoveredBackgroundColor());
        }
        else
        {
            painter->setBrush(ColorRepository::itemViewItemSelectedBackgroundColor());
        }
    }
    // 悬停
    else if (option->state & QStyle::State_MouseOver)
    {
        painter->setBrush(ColorRepository::itemViewItemHoveredBackgroundColor());
    }
    // 默认
    else
    {
        painter->setBrush(ColorRepository::itemViewItemBackgroundColor());
    }
}