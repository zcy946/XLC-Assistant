#include "GroupBoxStyleHelper.h"
#include "XlcStyle.h"
#include "ColorRepository.h"

void GroupBoxStyleHelper::drawBorder(const XlcStyle *style, const QStyleOptionGroupBox *option, QPainter *painter, const QWidget *widget) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(ColorRepository::groupBoxBorderColor(), WIDTH_BORDER));
    painter->setBrush(ColorRepository::groupBoxBackgroundColor());
    painter->drawRoundedRect(style->subControlRect(QStyle::CC_GroupBox, option, QStyle::SC_GroupBoxFrame, widget), RADIUS, RADIUS);
    painter->restore();
}

void GroupBoxStyleHelper::drawLabel(const XlcStyle *style, const QStyleOptionGroupBox *option, QPainter *painter, const QWidget *widget) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setBrush(Qt::NoBrush);
    QRect rectFrame = style->subControlRect(QStyle::CC_GroupBox, option, QStyle::SC_GroupBoxFrame, widget);
    QRect rectLabel = style->subControlRect(QStyle::CC_GroupBox, option, QStyle::SC_GroupBoxLabel, widget);
    // 绘制遮盖线
    painter->setPen(QPen(ColorRepository::groupBoxBackgroundColor(), WIDTH_BORDER));
    painter->drawLine(rectLabel.x() - PADDING_LABEL,
                      rectFrame.y(),
                      rectLabel.x() + option->fontMetrics.horizontalAdvance(option->text) + PADDING_LABEL,
                      rectFrame.y());
    // 绘制文本
    painter->setPen(ColorRepository::basicTextColor());
    painter->drawText(rectLabel, Qt::AlignVCenter, option->text);
    painter->restore();
}

QRect GroupBoxStyleHelper::rectFrame(const QStyleOptionGroupBox *option, const QWidget *widget) const
{
    Q_UNUSED(widget)
    return option->rect.adjusted(WIDTH_BORDER, WIDTH_BORDER + option->fontMetrics.height() / 2, -WIDTH_BORDER, -WIDTH_BORDER);
}

QRect GroupBoxStyleHelper::rectLabel(const QStyleOptionGroupBox *option, const QWidget *widget, QRect rectLabelOriginal) const
{
    Q_UNUSED(widget)
    return QRect(option->rect.x() + MARGIN_LABEL, rectLabelOriginal.y(), rectLabelOriginal.width(), rectLabelOriginal.height());
}

QRect GroupBoxStyleHelper::rectContents(const QStyleOptionGroupBox *option, const QWidget *widget, QRect rectContentsOriginal) const
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    return rectContentsOriginal;
}