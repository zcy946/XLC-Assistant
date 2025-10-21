#include "CheckBoxStyleHelper.h"
#include <QCheckBox>
#include "ColorRepository.h"
#include "XlcStyle.h"

void CheckBoxStyleHelper::drawBackground(const XlcStyle *style, const QStyleOptionButton *option, QPainter *painter, const QWidget *widget) const
{
    QRect rectCheckIndicator = style->subElementRect(QStyle::SE_CheckBoxIndicator, option, widget);
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);
    if (option->state.testFlag(QStyle::State_On) || option->state.testFlag(QStyle::State_NoChange))
    {
        painter->setPen(Qt::NoPen);
        if (option->state.testFlag(QStyle::State_Sunken))
        {
            painter->setBrush(ColorRepository::checkBoxPressedBackgroundColor(true));
        }
        else
        {
            if (option->state.testFlag(QStyle::State_MouseOver))
            {
                painter->setBrush(ColorRepository::checkBoxHoveredBackgroundColor(true));
            }
            else
            {
                painter->setBrush(ColorRepository::checkBoxBackgroundColor(true));
            }
        }
    }
    else
    {
        if (option->state.testFlag(QStyle::State_Sunken))
        {
            painter->setPen(ColorRepository::checkBoxBorderColor());
            painter->setBrush(ColorRepository::checkBoxPressedBackgroundColor(false));
        }
        else
        {
            painter->setPen(ColorRepository::checkBoxBorderColor());
            if (option->state.testFlag(QStyle::State_MouseOver))
            {
                painter->setBrush(ColorRepository::checkBoxHoveredBackgroundColor(false));
            }
            else
            {
                painter->setBrush(ColorRepository::checkBoxBackgroundColor(false));
            }
        }
    }
    // 绘制复选框背景
    painter->drawRoundedRect(rectCheckIndicator, RADIUS, RADIUS);
    painter->restore();
}

void CheckBoxStyleHelper::drawMarkIndicator(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget, QRect rectIndicatorOriginal) const
{
    Q_UNUSED(widget)
    QRect rectCheckIndicator = option->rect.adjusted(WIDTH_BORDER, WIDTH_BORDER, -WIDTH_BORDER, -WIDTH_BORDER);
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(ColorRepository::checkBoxIndicatorColor());
    if (option->state.testFlag(QStyle::State_On))
    {
        painter->save();
        QFont iconFont = QFont(ICONFONT_NAME);
        iconFont.setPixelSize(rectCheckIndicator.width() * 0.75);
        painter->setFont(iconFont);
        painter->drawText(rectCheckIndicator, Qt::AlignCenter, ICONFONT_Check);
        painter->restore();
    }
    else if (option->state.testFlag(QStyle::State_NoChange))
    {
        int widthMarkLine = rectCheckIndicator.width() / 10 * 8;
        int x = rectCheckIndicator.x() + (rectCheckIndicator.width() - widthMarkLine) / 2;
        int y = rectCheckIndicator.center().y();
        QLine lineCheck(x, y, x + widthMarkLine, y);
        painter->drawLine(lineCheck);
    }
    painter->restore();
}

void CheckBoxStyleHelper::drawText(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget) const
{
    Q_UNUSED(widget)
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    if (option->state.testFlag(QStyle::State_Enabled))
    {
        painter->setPen(ColorRepository::basicTextColor());
    }
    else
    {
        painter->setPen(ColorRepository::basicDisableText());
    }
    // QRect rectText = QRect(option->rect.x() + WIDTH_INDICATOR + SPACING_CHECKBOX_TO_LABEL, ❌
    QRect rectText = QRect(option->rect.x(), // ✔ 无需手动计算，因为qt内部调用时传入的是已经将减去 指示器及其间距所占的宽度 的剩余矩形区域
                           option->rect.y(),
                           option->fontMetrics.horizontalAdvance(option->text),
                           option->fontMetrics.height());
    painter->drawText(rectText, option->text);
    painter->restore();
}

int CheckBoxStyleHelper::spacingIndicatorToLabel() const
{
    return SPACING_CHECKBOX_TO_LABEL;
}

int CheckBoxStyleHelper::widthIndicator() const
{
    return WIDTH_INDICATOR;
}

int CheckBoxStyleHelper::heightIndicator() const
{
    return HEIGHT_INDICATOR;
}

QRect CheckBoxStyleHelper::rectContents(const XlcStyle *style, const QStyleOptionButton *option, const QWidget *widget) const
{
    int widthCheckIndicator = style->pixelMetric(QStyle::PM_IndicatorWidth, option, widget);
    int heightCheckIndicator = style->pixelMetric(QStyle::PM_IndicatorHeight, option, widget);
    int spacingCheckBoxToLabel = style->pixelMetric(QStyle::PM_CheckBoxLabelSpacing, option, widget);

    if (option->text.isEmpty() || option->text.isNull())
    {
        return QRect(option->rect.x(), option->rect.y(), widthCheckIndicator, heightCheckIndicator);
    }
    int widthText = option->fontMetrics.horizontalAdvance(option->text);
    return QRect(option->rect.x(), option->rect.y(), widthCheckIndicator + spacingCheckBoxToLabel + widthText, heightCheckIndicator);
}

QRect CheckBoxStyleHelper::rectCheckIndicator(const QStyleOptionButton *option, const QWidget *widget) const
{
    QRect rectCheckIndicator;
    // QCheckBox中的可选框有特殊大小
    if (qobject_cast<const QCheckBox *>(widget))
    {
        rectCheckIndicator = QRect(option->rect.x(), option->rect.y(), WIDTH_INDICATOR, HEIGHT_INDICATOR);
        rectCheckIndicator.adjust(WIDTH_BORDER, WIDTH_BORDER, -WIDTH_BORDER, -WIDTH_BORDER);
    }
    else
    {
        rectCheckIndicator = option->rect.adjusted(WIDTH_BORDER, WIDTH_BORDER, -WIDTH_BORDER, -WIDTH_BORDER);
    }
    return rectCheckIndicator;
}