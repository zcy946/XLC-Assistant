#include "XlcStyle.h"
#include <QPushButton>
#include <QCheckBox>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QTimer>
#include <QPointer>
#include <QFontDatabase>
#include <QListView>
#include <QPlainTextEdit>
#include <QDebug>

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
      m_spinBoxStyleHelper(new SpinBoxStyleHelper()),
      m_plainTextEditStyleHelper(new PlainTextEditStyleHelper()),
      m_checkBoxStyleHelper(new CheckBoxStyleHelper())
{
    QFontDatabase::addApplicationFont("://font/ElaAwesome.ttf");
}

void XlcStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (pe)
    {
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
    case PE_IndicatorCheckBox:
        if (const QStyleOptionButton *optionButton = qstyleoption_cast<const QStyleOptionButton *>(option))
        {
            m_checkBoxStyleHelper->drawBackground(this, optionButton, painter, widget);
            m_checkBoxStyleHelper->drawMarkIndicator(optionButton, painter, widget, QProxyStyle::subElementRect(SE_CheckBoxIndicator, optionButton, widget));
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
            // 绘制背景
            m_itemViewItemStyleHelper->drawBackground(optionItemViewItem, painter, widget);
            // 绘制复选框
            m_itemViewItemStyleHelper->drawCheckIndicator(this, optionItemViewItem, painter, widget);
            // 绘制文本
            m_itemViewItemStyleHelper->drawText(this, optionItemViewItem, painter, widget);
            // 绘制(选中)标记
            m_itemViewItemStyleHelper->drawMarket(optionItemViewItem, painter, widget);
            return;
        }
        break;
    case CE_ShapedFrame:
        if (const QStyleOptionFrame *optionFrame = qstyleoption_cast<const QStyleOptionFrame *>(option))
        {
            if (qobject_cast<const QListView *>(widget))
            {
                m_listViewStyleHelper->drawBorder(painter, optionFrame->rect);
                return;
            }
            if (qobject_cast<const QPlainTextEdit *>(widget))
            {
                m_plainTextEditStyleHelper->drawBackgroundAndBorder(optionFrame, painter, widget);
                m_plainTextEditStyleHelper->drawHemline(optionFrame, painter, widget);
                return;
            }
        }
        break;
    case CE_CheckBoxLabel:
        if (const QStyleOptionButton *optionButton = qstyleoption_cast<const QStyleOptionButton *>(option))
        {
            m_checkBoxStyleHelper->drawText(optionButton, painter, widget);
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
            break;
        }
        break;
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
        // 按钮按下时的水平和垂直内容偏移
        return 0; // 不偏移
    case PM_CheckBoxLabelSpacing:
        if (qstyleoption_cast<const QStyleOptionButton *>(option))
        {
            if (qobject_cast<const QCheckBox *>(widget))
            {
                return m_checkBoxStyleHelper->spacingIndicatorToLabel();
            }
            break;
        }
        break;
    case PM_IndicatorWidth:
        if (qstyleoption_cast<const QStyleOptionButton *>(option))
        {
            if (qobject_cast<const QCheckBox *>(widget))
            {
                return m_checkBoxStyleHelper->widthIndicator();
            }
        }
        break;
    case PM_IndicatorHeight:
        if (qstyleoption_cast<const QStyleOptionButton *>(option))
        {
            if (qobject_cast<const QCheckBox *>(widget))
            {
                return m_checkBoxStyleHelper->heightIndicator();
            }
        }
        break;
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
    case SE_CheckBoxIndicator:
        if (const QStyleOptionButton *optionButton = qstyleoption_cast<const QStyleOptionButton *>(option))
        {
            return m_checkBoxStyleHelper->rectCheckIndicator(optionButton, widget);
        }
        break;
    case SE_ItemViewItemCheckIndicator:
        if (const QStyleOptionViewItem *optionViewItem = qstyleoption_cast<const QStyleOptionViewItem *>(option))
        {
            return m_itemViewItemStyleHelper->rectCheckIndicator(optionViewItem, widget);
        }
        break;
    case SE_CheckBoxContents:
    case SE_CheckBoxClickRect:
        if (const QStyleOptionButton *optionButton = qstyleoption_cast<const QStyleOptionButton *>(option))
        {
            return m_checkBoxStyleHelper->rectContents(this, optionButton, widget);
        }
        if (const QStyleOptionViewItem *optionViewItem = qstyleoption_cast<const QStyleOptionViewItem *>(option))
        {
            return m_itemViewItemStyleHelper->rectClickCheckIndicator(this, optionViewItem, widget);
        }
        break;
    case SE_LineEditContents:
        if (const QStyleOptionFrame *optionLineEdit = qstyleoption_cast<const QStyleOptionFrame *>(option))
        {
            return m_lineEditStyleHelper->subElementRect(subElement, optionLineEdit, widget,
                                                         QProxyStyle::subElementRect(subElement, optionLineEdit, widget));
        }
        break;
    case SE_FrameContents:
    {
        if (const QStyleOptionFrame *optionFrame = qstyleoption_cast<const QStyleOptionFrame *>(option))
        {
            // 获取内容绘制区域
            QRect rectFinal = m_plainTextEditStyleHelper->contentArea(optionFrame, widget);
            if (!rectFinal.isValid())
                break;
            return rectFinal;
        }
        break;
    }
    case SE_ItemViewItemText:
        if (const QStyleOptionViewItem *optionViewItem = qstyleoption_cast<const QStyleOptionViewItem *>(option))
        {
            return m_itemViewItemStyleHelper->rectText(optionViewItem, widget, QProxyStyle::subElementRect(subElement, option, widget));
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

    // NOTE 可以在 polish 中判断目标类型，从而替换调色板实现不同颜色

    // 阴影绘制
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
