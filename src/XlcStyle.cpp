#include "XlcStyle.h"
#include <QPushButton>
#include <QCheckBox>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QTimer>
#include <QPointer>
#include <QFontDatabase>
#include <QListView>

/**
 * Note 在重写任何 QProxyStyle 虚函数时，都要遵循这张清单:
 *
 *  1. 检查 enum 参数: 这是你的第一道防线。检查传入的 PrimitiveElement, ControlElement, PixelMetric, StyleHint 等是否是你感兴趣的那个。
 *  2. 检查 QStyleOption 类型: 如果你需要访问特定于控件的额外信息（如按钮的文本、边框的宽度），必须使用 qstyleoption_cast 进行安全的类型转换，并检查返回值是否为 nullptr。
 *  3. 检查 QWidget 类型: 这是最关键的检查之一。使用 qobject_cast 确保你正在为预期的控件（如 QLineEdit, QPushButton）执行代码。这能防止你的样式“污染”了其他无辜的控件。
 *  4. 检查 QStyleOption::state: 如果你的自定义逻辑只在特定状态下（如 State_HasFocus, State_ReadOnly, State_On）生效，务必检查 option->state 中的标志位。
 */

XlcStyle::XlcStyle()
    : QProxyStyle(),
      m_pushButtonStyleHelper(new PushButtonStyleHelper()),
      m_listViewStyleHelper(new ListViewStyleHelper),
      m_itemViewItemStyleHelper(new ItemViewItemStyleHelper()),
      m_scrollBarStyleHelper(new ScrollBarStyleHelper()),
      m_lineEditStyleHelper(new LineEditStyleHelper()),
      m_spinBoxStyleHelper(new SpinBoxStyleHelper())
{
    QFontDatabase::addApplicationFont("://font/ElaAwesome.ttf");
}

void XlcStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (pe)
    {
    case PE_Frame:
        if (qobject_cast<const QListView *>(widget))
        {
            m_listViewStyleHelper->drawBorder(painter, option->rect);
            return;
        }
        break;
    case PE_FrameFocusRect:
        // 不绘制虚线焦点框
        return;
    case PE_PanelLineEdit:
        if (const QStyleOptionFrame *optionLineEdit = qstyleoption_cast<const QStyleOptionFrame *>(option))
        {
            m_lineEditStyleHelper->drawLineEditShape(optionLineEdit, painter, widget);
            return;
        }
        break;
    default:
        break;
    }
    QProxyStyle::drawPrimitive(pe, option, painter, widget);
}
void XlcStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element)
    {
    case CE_PushButtonBevel:
        // 绘制按钮形状（背景与边框）
        if (const QStyleOptionButton *optionButton = qstyleoption_cast<const QStyleOptionButton *>(option))
        {
            m_pushButtonStyleHelper->drawButtonShape(optionButton, painter, widget);
            return;
        }
        break;
    case CE_PushButtonLabel:
        // 绘制按钮文本、图标（及菜单指示器）
        if (const QStyleOptionButton *optionButton = qstyleoption_cast<const QStyleOptionButton *>(option))
        {
            m_pushButtonStyleHelper->drawText(optionButton, painter, widget);
            return;
        }
        break;
    case CE_ItemViewItem:
        if (const QStyleOptionViewItem *optionItemViewItem = qstyleoption_cast<const QStyleOptionViewItem *>(option))
        {
            m_itemViewItemStyleHelper->drawBackground(optionItemViewItem, painter, widget);
            QRect rectTextOriginal = QProxyStyle::subElementRect(QStyle::SE_ItemViewItemText, option, widget);
            m_itemViewItemStyleHelper->drawText(optionItemViewItem, painter, widget, rectTextOriginal);
            // 绘制(选中)标记
            m_itemViewItemStyleHelper->drawMarket(optionItemViewItem, painter, widget);

            QStyleOptionViewItem optionNew = *optionItemViewItem;
            // 清除文本，防止重复绘制
            optionNew.text.clear();
            // 删除selected状态
            if (optionNew.state & State_Selected)
            {
                optionNew.state &= ~State_Selected;
            }
            // 删除hovered状态
            if (optionNew.state & State_MouseOver)
            {
                optionNew.state &= ~State_MouseOver;
            }
            // 调用基类绘制其他元素
            QProxyStyle::drawControl(element, &optionNew, painter, widget);
            return;
        }
        break;
    default:
        break;
    }
    QProxyStyle::drawControl(element, option, painter, widget);
}

void XlcStyle::drawComplexControl(ComplexControl complexControl, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    switch (complexControl)
    {
    case QStyle::CC_ScrollBar:
        if (const QStyleOptionSlider *optionSlider = qstyleoption_cast<const QStyleOptionSlider *>(option))
        {
            m_scrollBarStyleHelper->drawBackground(optionSlider, painter, widget);
            m_scrollBarStyleHelper->drawGroove(optionSlider, painter, widget,
                                               QProxyStyle::subControlRect(QStyle::CC_ScrollBar, option, QStyle::SC_ScrollBarGroove, widget));
            m_scrollBarStyleHelper->drawSlider(optionSlider, painter, widget,
                                               QProxyStyle::subControlRect(QStyle::CC_ScrollBar, option, QStyle::SC_ScrollBarSlider, widget));
            m_scrollBarStyleHelper->drawSubControls(optionSlider, painter, widget,
                                                    QProxyStyle::subControlRect(QStyle::CC_ScrollBar, option, QStyle::SC_ScrollBarSubLine, widget),
                                                    QProxyStyle::subControlRect(QStyle::CC_ScrollBar, option, QStyle::SC_ScrollBarAddLine, widget));
            return;
        }
        break;
    case QStyle::CC_SpinBox:
        if (const QStyleOptionSpinBox *optionSpinBox = qstyleoption_cast<const QStyleOptionSpinBox *>(option))
        {
            m_spinBoxStyleHelper->drawBackground(optionSpinBox, painter, widget);
            m_spinBoxStyleHelper->drawSubControls(optionSpinBox, painter, widget,
                                                  QProxyStyle::subControlRect(complexControl, optionSpinBox, QStyle::SC_ScrollBarSubLine, widget),
                                                  QProxyStyle::subControlRect(complexControl, optionSpinBox, QStyle::SC_ScrollBarAddLine, widget));
            m_spinBoxStyleHelper->drawHemline(optionSpinBox, painter, widget);
            return;
        }
        break;
    default:
        break;
    }
    QProxyStyle::drawComplexControl(complexControl, option, painter, widget);
}

int XlcStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric)
    {
    case PM_ButtonMargin:
        if (qstyleoption_cast<const QStyleOptionButton *>(option))
        {
            if (qobject_cast<const QPushButton *>(widget))
            {
                return m_pushButtonStyleHelper->padding();
            }
        }
        break;
    case PM_ScrollBarExtent:
        // scrollbar宽/高度
        if (qstyleoption_cast<const QStyleOptionSlider *>(option))
        {
            if (qobject_cast<const QScrollBar *>(widget))
            {
                return m_scrollBarStyleHelper->scrollBarExtent();
            }
        }
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
        // 按钮按下时的水平和垂直内容偏移
        return 0; // 不偏移
    default:
        break;
    }
    return QProxyStyle::pixelMetric(metric, option, widget);
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
        if (const QStyleOptionButton *buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option))
        {
            return m_pushButtonStyleHelper->sizeFromContents(buttonOption, contentsSize, widget);
        }
        break;
    case CT_LineEdit:
        if (const QStyleOptionFrame *lineEditOption = qstyleoption_cast<const QStyleOptionFrame *>(option))
        {
            QSize sizeBasic = QProxyStyle::sizeFromContents(type, lineEditOption, contentsSize, widget);
            return m_lineEditStyleHelper->sizeFromContents(lineEditOption, sizeBasic, widget);
        }
        break;
    case CT_ItemViewItem:
        if (const QStyleOptionViewItem *optionItemViewItem = qstyleoption_cast<const QStyleOptionViewItem *>(option))
        {
            // 让 helper 在父类尺寸基础上调整
            QSize sizeOriginal = QProxyStyle::sizeFromContents(type, optionItemViewItem, contentsSize, widget);
            return m_itemViewItemStyleHelper->sizeFromContents(optionItemViewItem, sizeOriginal, widget);
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
    case SE_LineEditContents:
        if (const QStyleOptionFrame *optionLineEdit = qstyleoption_cast<const QStyleOptionFrame *>(option))
        {
            return m_lineEditStyleHelper->subElementRect(subElement, optionLineEdit, widget,
                                                         QProxyStyle::subElementRect(subElement, optionLineEdit, widget));
        }
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
