#ifndef CHECKBOXSTYLEHELPER_H
#define CHECKBOXSTYLEHELPER_H

#include <QStyleOptionButton>
#include <QPainter>

class XlcStyle;
class CheckBoxStyleHelper
{
public:
    // 绘制背景
    void drawBackground(const XlcStyle *style, const QStyleOptionButton *option, QPainter *painter, const QWidget *widget) const;
    // 绘制复选框中的状态图标
    void drawMarkIndicator(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget, const QRect &rectIndicatorOriginal) const;
    // 绘制文本
    void drawText(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget) const;
    // 复选框到文本之间的距离
    int spacingIndicatorToLabel() const;
    // 复选框宽
    int widthIndicator() const;
    // 复选框高
    int heightIndicator() const;
    // 指示器+文本的区域
    QRect rectContents(const XlcStyle *style, const QStyleOptionButton *option, const QWidget *widget) const;
    // 复选框区域
    QRect rectCheckIndicator(const QStyleOptionButton *option, const QWidget *widget) const;

private:
    const int WIDTH_INDICATOR = 21;             // 复选框指示器的宽
    const int HEIGHT_INDICATOR = 21;            // 复选框指示器的高
    const int WIDTH_BORDER = 1;                 // 复选框指示器边框宽度
    const int RADIUS = 2;                       // 复选框指示器圆角半径
    const QString ICONFONT_NAME = "ElaAwesome"; // 字体图标名称
    const QChar ICONFONT_Check = QChar(0xea6c); // 勾选图标
    const int SPACING_CHECKBOX_TO_LABEL = 10;   // 复选框到文本之间的距离
};

#endif // CHECKBOXSTYLEHELPER_H