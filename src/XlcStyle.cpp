#include "XlcStyle.h"
#include <QPushButton>
#include <QCheckBox>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QTimer>
#include <QPointer>

XlcStyle::XlcStyle()
    : QProxyStyle(), m_pushButtonStyleHelper(new PushButtonStyleHelper()), m_itemViewItemStyleHelper(new ItemViewItemStyleHelper()),
      m_scrollBarStyleHelper(new ScrollBarStyleHelper())
{
}

void XlcStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (pe)
    {
    case PE_FrameFocusRect:
        // 虚线焦点框
        break;
    default:
        QProxyStyle::drawPrimitive(pe, option, painter, widget);
        break;
    }
}
void XlcStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element)
    {
    case CE_PushButton:
        // 使用默认实现
        // 以及 PE_FrameFocusRect（我们已将其重实现为空操作）。
        QProxyStyle::drawControl(element, option, painter, widget);
        return;
    case CE_PushButtonBevel:
        // 绘制按钮形状（背景与边框）
        if (const QStyleOptionButton *optionButton = qstyleoption_cast<const QStyleOptionButton *>(option))
        {
            m_pushButtonStyleHelper->drawButtonShape(optionButton, painter, widget);
        }
        return;
    case CE_PushButtonLabel:
        // 绘制按钮文本、图标（及菜单指示器）
        if (const QStyleOptionButton *optionButton = qstyleoption_cast<const QStyleOptionButton *>(option))
        {
            m_pushButtonStyleHelper->drawText(optionButton, painter, widget);
        }
        return;

    case CE_RadioButton: // 仅调用 PE_IndicatorRadioButton、CE_RadioButtonLabel（及焦点框）
    case CE_RadioButtonLabel:
    case CE_CheckBox: // 仅调用 PE_IndicatorCheckBox、CE_CheckBoxLabel（及焦点框）
    case CE_CheckBoxLabel:
        QProxyStyle::drawControl(element, option, painter, widget);
        return;

    case CE_ProgressBar: // 主入口点
        // 调用 CE_ProgressBarGroove、CE_ProgressBarContents 和 CE_ProgressBarLabel
        QProxyStyle::drawControl(element, option, painter, widget);
        return;
    case CE_ProgressBarGroove:
        break;
    case CE_ProgressBarContents:
        break;
    case CE_ProgressBarLabel:
        break;
    case CE_ItemViewItem:
        if (const QStyleOptionViewItem *optionItemViewItem = qstyleoption_cast<const QStyleOptionViewItem *>(option))
        {
            m_itemViewItemStyleHelper->drawItemViewItemShape(optionItemViewItem, painter, widget);
            m_itemViewItemStyleHelper->drawText(optionItemViewItem, painter, widget);
            // 绘制(选中)标记
            m_itemViewItemStyleHelper->drawMarket(optionItemViewItem, painter, widget);
        }
        break;

    default:
        QProxyStyle::drawControl(element, option, painter, widget);
    }
}

void XlcStyle::drawComplexControl(ComplexControl complexControl, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    switch (complexControl)
    {
    case QStyle::CC_ScrollBar:
        if (const QStyleOptionSlider *sliderOption = qstyleoption_cast<const QStyleOptionSlider *>(option))
        {
            m_scrollBarStyleHelper->drawScrollBarShapes(sliderOption, painter, widget, this);
        }
        break;
    default:
        QProxyStyle::drawComplexControl(complexControl, option, painter, widget);
        break;
    }
}

int XlcStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric)
    {
    case PM_ButtonMargin:
        if (qstyleoption_cast<const QStyleOptionButton *>(option))
        {
            return m_pushButtonStyleHelper->padding();
        }
        return QProxyStyle::pixelMetric(metric, option, widget);
    case PM_ScrollBarExtent:
        return m_scrollBarStyleHelper->scrollBarExtent();
    case PM_IndicatorHeight: // checkboxes
    case PM_IndicatorWidth:  // checkboxes
        break;
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
        // 按钮按下时的水平和垂直内容偏移
        return 0; // 不偏移
    default:
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
}

int XlcStyle::styleHint(StyleHint stylehint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    switch (stylehint)
    {
    case SH_DialogButtonBox_ButtonsHaveIcons:
        return 0;
    default:
        break;
    }

    return QProxyStyle::styleHint(stylehint, option, widget, returnData);
}

QSize XlcStyle::sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &contentsSize, const QWidget *widget) const
{
    switch (type)
    {
    case CT_PushButton:
        if (const auto *buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option))
        {
            return m_pushButtonStyleHelper->sizeFromContents(buttonOption, contentsSize, widget);
        }
        break;
    case CT_RadioButton:
    case CT_CheckBox:
        return QProxyStyle::sizeFromContents(type, option, contentsSize, widget);
    case CT_ItemViewItem:
        if (const auto *optionItemViewItem = qstyleoption_cast<const QStyleOptionViewItem *>(option))
        {
            return m_itemViewItemStyleHelper->sizeFromContents(optionItemViewItem, contentsSize, widget);
        }
        break;
    default:
        break;
    }
    return QProxyStyle::sizeFromContents(type, option, contentsSize, widget);
}

QRect XlcStyle::subElementRect(SubElement subElement, const QStyleOption *option, const QWidget *widget) const
{
    switch (subElement)
    {
    case SE_ProgressBarGroove:
    case SE_ProgressBarContents:
    case SE_ProgressBarLabel:
        // if (const auto *progressBarOption = qstyleoption_cast<const QStyleOptionProgressBar *>(option))
        // {
        //     return mProgressBarStyleHelper->subElementRect(subElement, progressBarOption, widget);
        // }
        break;
    default:
        break;
    }
    return QProxyStyle::subElementRect(subElement, option, widget);
}

QRect XlcStyle::subControlRect(ComplexControl complexControl, const QStyleOptionComplex *option, SubControl subControl, const QWidget *widget) const
{
    return QProxyStyle::subControlRect(complexControl, option, subControl, widget);
}

void XlcStyle::polish(QWidget *w)
{
    if (qobject_cast<QPushButton *>(w) || qobject_cast<QCheckBox *>(w) || qobject_cast<QAbstractItemView *>(w) || qobject_cast<QScrollBar *>(w))
    {
        w->setAttribute(Qt::WA_Hover);
    }
    // // 为QPushButton绘制阴影
    // if (QPushButton *button = qobject_cast<QPushButton *>(w))
    // {
    //     if (!button->isVisible())
    //     {
    //         QTimer::singleShot(0, button, [this, button]()
    //                            { m_pushButtonStyleHelper->drawShadow(button); });
    //     }
    //     else
    //     {
    //         m_pushButtonStyleHelper->drawShadow(button);
    //     }
    // }
    // 修复阴影绘制的时序问题
    if (QPushButton *button = qobject_cast<QPushButton *>(w))
    {
        if (!button->isVisible())
        {
            // 使用 QPointer 防止悬空指针
            QPointer<QPushButton> safeButton(button);
            QTimer::singleShot(0, this,
                               [this, safeButton]()
                               {
                                   if (safeButton)
                                   { 
                                    // 检查指针是否仍然有效
                                       m_pushButtonStyleHelper->drawShadow(safeButton);
                                   }
                               });
        }
        else
        {
            m_pushButtonStyleHelper->drawShadow(button);
        }
    }
    QProxyStyle::polish(w);
}

bool XlcStyle::eventFilter(QObject *obj, QEvent *event)
{
    return QProxyStyle::eventFilter(obj, event);
}
