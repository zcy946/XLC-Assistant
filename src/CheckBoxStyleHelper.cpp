#include "CheckBoxStyleHelper.h"
#include <QCheckBox>
#include "ColorRepository.h"
#include <QListView>

void CheckBoxStyleHelper::drawBackground(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget) const
{
    Q_UNUSED(widget)
    QRect rect = option->rect;
    QRect rectIndicator(rect.x(), rect.y(), WIDTH_INDICATOR, HEIGHT_INDICATOR);
    rectIndicator.adjust(WIDTH_BORDER, WIDTH_BORDER, -WIDTH_BORDER, -WIDTH_BORDER);
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
    painter->drawRoundedRect(rectIndicator, RADIUS, RADIUS);
    painter->restore();
}

void CheckBoxStyleHelper::drawIndicator(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget, const QRect &rectIndicatorOriginal) const
{
    Q_UNUSED(widget)
    QRect rectIndicator = rectIndicatorOriginal.adjusted(WIDTH_BORDER, WIDTH_BORDER, -WIDTH_BORDER, -WIDTH_BORDER);
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(ColorRepository::checkBoxIndicatorColor());
    if (option->state.testFlag(QStyle::State_On))
    {
        painter->save();
        QFont iconFont = QFont(ICONFONT_NAME);
        iconFont.setPixelSize(WIDTH_INDICATOR * 0.75);
        painter->setFont(iconFont);
        painter->drawText(rectIndicator, Qt::AlignCenter, ICONFONT_Check);
        painter->restore();
    }
    else if (option->state.testFlag(QStyle::State_NoChange))
    {
        QLine lineCheck(rectIndicator.x() + 3, rectIndicator.center().y(), rectIndicator.right() - 3, rectIndicator.center().y());
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
        painter->setPen(ColorRepository::basicText());
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

QRect CheckBoxStyleHelper::rectContents(const QStyleOptionButton *option, const QWidget *widget)
{
    Q_UNUSED(widget)
    int widthText = option->fontMetrics.horizontalAdvance(option->text);
    return QRect(option->rect.x(), option->rect.y(), WIDTH_INDICATOR + SPACING_CHECKBOX_TO_LABEL + widthText, HEIGHT_INDICATOR);
}

QRect CheckBoxStyleHelper::rectIndicator(const QStyleOptionButton *option, const QWidget *widget)
{
    Q_UNUSED(widget)
    return QRect(option->rect.x(), option->rect.y(), WIDTH_INDICATOR, HEIGHT_INDICATOR);
}