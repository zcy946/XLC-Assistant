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
#include <QComboBox>
#include <QMenu>

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
      m_checkBoxStyleHelper(new CheckBoxStyleHelper()),
      m_comboBoxStyleHelper(new ComboBoxStyleHelper()),
      m_groupBoxStyleHelper(new GroupBoxStyleHelper()),
      m_menuStyleHelper(new MenuStyleHelper())
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
    case PE_IndicatorItemViewItemCheck:
        if (const QStyleOptionViewItem *optionItemViewItem = qstyleoption_cast<const QStyleOptionViewItem *>(option))
        {
            m_itemViewItemStyleHelper->drawCheckIndicator(this, optionItemViewItem, painter, widget);
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
    case PE_PanelItemViewItem:
        if (const QStyleOptionViewItem *optionItemViewItem = qstyleoption_cast<const QStyleOptionViewItem *>(option))
        {
            // 绘制背景
            m_itemViewItemStyleHelper->drawBackground(optionItemViewItem, painter, widget);
            // 绘制(选中)标记
            m_itemViewItemStyleHelper->drawMarket(optionItemViewItem, painter, widget);
            return;
        }
        break;
    case PE_PanelMenu:
        if (const QStyleOptionMenuItem *optionItemMenuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(option))
        {
            m_menuStyleHelper->drawBackground(optionItemMenuItem, painter, widget);
            // TODO 修改Menu的背景添加自定义阴影(如何去除自带的阴影)
            // m_menuStyleHelper->drawShadow(optionItemMenuItem, painter, widget);
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
            m_pushButtonStyleHelper->drawShadow(optionButton, painter, widget);
            m_pushButtonStyleHelper->drawBackground(optionButton, painter, widget);
            m_pushButtonStyleHelper->drawHemline(optionButton, painter, widget);
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
    case CE_MenuItem:
        if (const QStyleOptionMenuItem *optionMenuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(option))
        {
            m_menuStyleHelper->drawBackgroundItem(optionMenuItem, painter, widget);
            m_menuStyleHelper->drawSeparator(optionMenuItem, painter, widget);
            m_menuStyleHelper->drawCheckIndicator(optionMenuItem, painter, widget);
            m_menuStyleHelper->drawText(optionMenuItem, painter, widget);
            m_menuStyleHelper->drawIcon(QProxyStyle::pixelMetric(QStyle::PM_SmallIconSize, optionMenuItem, widget),
                                        optionMenuItem, painter, widget);
            m_menuStyleHelper->drawIconExpand(optionMenuItem, painter, widget);
            return;
        }
        break;
    case CE_ComboBoxLabel:
        if (const QStyleOptionComboBox *optionComboBox = qstyleoption_cast<const QStyleOptionComboBox *>(option))
        {
            m_comboBoxStyleHelper->drawText(optionComboBox, painter, widget);
            return;
        }
        break;
    case CE_ItemViewItem:
        if (const QStyleOptionViewItem *optionItemViewItem = qstyleoption_cast<const QStyleOptionViewItem *>(option))
        {
            // 绘制背景
            drawPrimitive(QStyle::PE_PanelItemViewItem, optionItemViewItem, painter, widget);
            // 绘制复选框
            drawPrimitive(QStyle::PE_IndicatorItemViewItemCheck, optionItemViewItem, painter, widget);
            // 绘制文本
            m_itemViewItemStyleHelper->drawText(this, optionItemViewItem, painter, widget);
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
        // QComboBox的菜单容器
        if (widget->objectName() == "XlcComboBoxContainer")
        {
            m_comboBoxStyleHelper->drawContainerBackground(option, painter, widget);
            return;
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
    case CC_SpinBox:
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
    case CC_ComboBox:
    {
        // 主体显示绘制
        if (const QStyleOptionComboBox *optionComboBox = qstyleoption_cast<const QStyleOptionComboBox *>(option))
        {
            m_comboBoxStyleHelper->drawBackground(this, optionComboBox, painter, widget);
            m_comboBoxStyleHelper->drawHemline(optionComboBox, painter, widget);
            m_comboBoxStyleHelper->drawArrow(this, optionComboBox, painter, widget);
            return;
        }
        break;
    }
    break;
    case CC_ScrollBar:
        if (const QStyleOptionSlider *optionSlider = qstyleoption_cast<const QStyleOptionSlider *>(option))
        {
            m_scrollBarStyleHelper->drawBackground(optionSlider, painter, widget);
            // TODO 查看是否能单拉出基类rect
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
    case CC_GroupBox:
        if (const QStyleOptionGroupBox *optionGroupBox = qstyleoption_cast<const QStyleOptionGroupBox *>(option))
        {
            m_groupBoxStyleHelper->drawBorder(this, optionGroupBox, painter, widget);
            m_groupBoxStyleHelper->drawLabel(this, optionGroupBox, painter, widget);
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
    case PM_MenuPanelWidth:
    case PM_MenuHMargin:
    case PM_MenuVMargin:
        return 0;
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
        if (const QStyleOptionButton *optionButton = qstyleoption_cast<const QStyleOptionButton *>(option))
        {
            return m_pushButtonStyleHelper->sizeFromContents(optionButton, contentsSize, widget);
        }
        break;
    case CT_ComboBox:
        if (const QStyleOptionComboBox *optionComboBox = qstyleoption_cast<const QStyleOptionComboBox *>(option))
        {
            return m_comboBoxStyleHelper->sizeFromContents(optionComboBox, contentsSize, widget);
        }
        break;
    case CT_MenuItem:
        if (const QStyleOptionMenuItem *optionMenuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(option))
        {
            return m_menuStyleHelper->sizeMenuItem(optionMenuItem, contentsSize, widget);
        }
        break;
    case CT_LineEdit:
        if (const QStyleOptionFrame *lineEditOption = qstyleoption_cast<const QStyleOptionFrame *>(option))
        {
            return m_lineEditStyleHelper->rectAll(lineEditOption, widget, QProxyStyle::sizeFromContents(type, lineEditOption, contentsSize, widget));
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
        // BUG QComboBox可选时没有绘制展开按钮
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
            return m_lineEditStyleHelper->rectText(optionLineEdit, widget, QProxyStyle::subElementRect(subElement, optionLineEdit, widget));
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
    switch (complexControl)
    {
    case CC_ComboBox:
    {
        switch (subControl)
        {
        case SC_ComboBoxFrame:
            if (const QStyleOptionComboBox *optionComBoBox = qstyleoption_cast<const QStyleOptionComboBox *>(option))
            {
                return m_comboBoxStyleHelper->rectFrame(optionComBoBox, widget,
                                                        QProxyStyle::subControlRect(QStyle::CC_ComboBox, optionComBoBox,
                                                                                    QStyle::SC_ComboBoxEditField, widget));
            }
            break;
        case SC_ComboBoxEditField:
            if (const QStyleOptionComboBox *optionComBoBox = qstyleoption_cast<const QStyleOptionComboBox *>(option))
            {
                return m_comboBoxStyleHelper->rectEditField(optionComBoBox, widget);
            }
            break;
        case SC_ComboBoxArrow:
            if (const QStyleOptionComboBox *optionComBoBox = qstyleoption_cast<const QStyleOptionComboBox *>(option))
            {
                return m_comboBoxStyleHelper->rectArrow(optionComBoBox, widget);
            }
            break;
        case SC_ComboBoxListBoxPopup:
            if (const QStyleOptionComboBox *optionComBoBox = qstyleoption_cast<const QStyleOptionComboBox *>(option))
            {
                return m_comboBoxStyleHelper->rectPopup(optionComBoBox, widget,
                                                        QProxyStyle::subControlRect(complexControl, option, subControl, widget));
            }
            break;
        default:
            break;
        }
        break;
    }
    case CC_GroupBox:
    {
        switch (subControl)
        {
        case SC_GroupBoxFrame:
            if (const QStyleOptionGroupBox *optionGroupBox = qstyleoption_cast<const QStyleOptionGroupBox *>(option))
            {
                return m_groupBoxStyleHelper->rectFrame(optionGroupBox, widget);
            }
            break;
        case SC_GroupBoxLabel:
            if (const QStyleOptionGroupBox *optionGroupBox = qstyleoption_cast<const QStyleOptionGroupBox *>(option))
            {
                return m_groupBoxStyleHelper->rectLabel(optionGroupBox, widget,
                                                        QProxyStyle::subControlRect(QStyle::CC_GroupBox, optionGroupBox, QStyle::SC_GroupBoxLabel, widget));
            }
            break;
        case SC_GroupBoxContents:
            if (const QStyleOptionGroupBox *optionGroupBox = qstyleoption_cast<const QStyleOptionGroupBox *>(option))
            {
                return m_groupBoxStyleHelper->rectContents(optionGroupBox, widget,
                                                           QProxyStyle::subControlRect(QStyle::CC_GroupBox, optionGroupBox, QStyle::SC_GroupBoxContents, widget));
            }
            break;
        default:
            break;
        }
    }
    default:
        break;
    }
    return QProxyStyle::subControlRect(complexControl, option, subControl, widget);
}

void XlcStyle::polish(QWidget *widget)
{
    if (qobject_cast<QPushButton *>(widget) || qobject_cast<QCheckBox *>(widget) || qobject_cast<QAbstractItemView *>(widget) || qobject_cast<QScrollBar *>(widget))
    {
        widget->setAttribute(Qt::WA_Hover);
    }

    if (QListView *listView = qobject_cast<QListView *>(widget))
    {
        // 通过私有类名`QComboBoxPrivateContainer`捕获QComboBox的菜单容器
        QWidget *popup = listView->parentWidget();
        if (popup && popup->inherits("QComboBoxPrivateContainer"))
        {
            popup->setAttribute(Qt::WA_TranslucentBackground, true);
            popup->setAttribute(Qt::WA_OpaquePaintEvent, false);
            popup->setAttribute(Qt::WA_NoSystemBackground, true);
            popup->setWindowFlag(Qt::FramelessWindowHint, true);
            popup->setWindowFlag(Qt::NoDropShadowWindowHint, true);
            popup->setObjectName("XlcComboBoxContainer");

            /**
             * BUG 绘制透明色会导致动画期间黑色
                // 增加外边距，为阴影绘制留空
                int widthShadowBorder = m_comboBoxStyleHelper->margin();
                popup->layout()->setContentsMargins(widthShadowBorder, 0, widthShadowBorder, widthShadowBorder);
             */
        }
    }

    if (QComboBox *comboBox = qobject_cast<QComboBox *>(widget))
    {
        // 安装自定义样式
        comboBox->setItemDelegate(sharedComboBoxDelegate());
    }

    /**
     * NOTE 可以在 polish 中判断目标类型，从而替换调色板实现不同颜色 */

    QProxyStyle::polish(widget);
}

bool XlcStyle::eventFilter(QObject *obj, QEvent *event)
{
    return QProxyStyle::eventFilter(obj, event);
}

ComboBoxDelegate *XlcStyle::sharedComboBoxDelegate()
{
    static ComboBoxDelegate delegate;
    return &delegate;
}

/**
 * ComboBoxDelegate
 */
ComboBoxDelegate::ComboBoxDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void ComboBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);
}