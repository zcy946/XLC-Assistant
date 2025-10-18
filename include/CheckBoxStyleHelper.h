#ifndef CHECKBOXSTYLEHELPER_H
#define CHECKBOXSTYLEHELPER_H

#include <QStyleOptionButton>
#include <QPainter>

class CheckBoxStyleHelper
{
public:
    void drawBackground(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget) const;
    void drawIndicator(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget, const QRect &rectIndicatorOriginal) const;
    void drawText(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget) const;
    int spacingIndicatorToLabel() const;
    int widthIndicator() const;
    int heightIndicator() const;
    // 指示器+文本的区域
    QRect rectContents(const QStyleOptionButton *option, const QWidget *widget);
    // 指示器区域
    QRect rectIndicator(const QStyleOptionButton *option, const QWidget *widget);

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