#ifndef COMBOBOXSTYLEHELPER_H
#define COMBOBOXSTYLEHELPER_H

#include <QStyleOptionComboBox>
#include <QPainter>

class XlcStyle;
class ComboBoxStyleHelper
{
public:
    // 绘制背景
    void drawBackground(const XlcStyle *style, const QStyleOptionComboBox *option, QPainter *painter, const QWidget *widget) const;
    // 绘制文本
    void drawText(const QStyleOptionComboBox *option, QPainter *painter, const QWidget *widget) const;
    // 绘制底部圆角矩形
    void drawHemline(const QStyleOptionComboBox *option, QPainter *painter, const QWidget *widget) const;
    // 绘制展开按钮
    void drawArrow(const XlcStyle *style, const QStyleOptionComboBox *option, QPainter *painter, const QWidget *widget) const;
    // 整个控件的区域
    QSize sizeFromContents(const QStyleOptionComboBox *option, QSize contentsSize, const QWidget *widget) const;
    // 边框区域(不包含展开按钮)
    QRect rectFrame(const QStyleOptionComboBox *option, const QWidget *widget, const QRect &rectFrameOriginal) const;
    // 文本编辑区域(设置了可修改属性)
    QRect rectEditField(const QStyleOptionComboBox *option, const QWidget *widget) const;
    // 展开按钮区域
    QRect rectArrow(const QStyleOptionComboBox *option, const QWidget *widget) const;

private:
    const int PADDING_HORIZONTAL = 12;              // 文本与边框之间的水平距离
    const int PADDING_VERTICAL = 6;                 // 文本与边框之间的垂直距离
    const int PADDING_ARROW = 6;                    // 下拉按钮图标外边距
    const int WIDTH_BORDER = 1;                     // 边框大小
    const int RADIUS = 3;                           // 背景圆角半径
    const int MARGIN_HEMLINE = 4;                   // 底部圆角矩形左下角外边距
    const int MARGIN_HEMLINE_FOCUSED = 6;           // 底部圆角矩形聚焦时左下角外边距
    const QString ICONFONT_NAME = "ElaAwesome";     // 字体图标名称
    const int ICONFONG_SIZE = 17;                   // 字体图标大小
    const QChar ICONFONT_AngleDown = QChar(0xe832); // 字体图标
};

#endif // COMBOBOXSTYLEHELPER_H