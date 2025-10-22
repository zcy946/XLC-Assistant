#ifndef TABBARSTYLEHELPER_H
#define TABBARSTYLEHELPER_H

#include <QStyleOptionTab>
#include <QPainter>

class XlcStyle;
class TabBarStyleHelper
{
public:
    void drawBackground(const QStyleOptionTab *option, QPainter *painter, const QWidget *widget) const;
    void drawLabel(const XlcStyle *style, const QStyleOptionTab *option, QPainter *painter, const QWidget *widget) const;
    QRect rectText(const QStyleOptionTab *option, const QWidget *widget, QRect rectTextOriginal) const;
    QSize sizeTab(const QStyleOptionTab *option, QSize sizeTabOriginal, const QWidget *widget) const;
    // 绘制QTabBarWidget背景
    void drawTabWidgetBackground(const QStyleOptionTabWidgetFrame *option, QPainter *painter, const QWidget *widget) const;

private:
    const int PADDING_HORIZONTAL = 12; // 水平内边距
    const int PADDING_VERTICAL = 3;    // 垂直内边距
    const int WIDTH_BORDER = 1;        // 边框宽度
    const int RADIUS = 6;              // 圆角半径
    const int RADIUS_TAB = 7;          // tab标签圆角半径
    const int MARGIN_TAB = 9;          // tab的外边用于绘制tab被选中时的圆角底边
    const int MARGIN_MARK = 7;         // 选中标记的外边距
    const int WIDTH_MARK = 3;          // 选中标记的宽度
    const int RADIUS_MARK = 2;         // 选中标记的圆角半径
};

#endif // TABBARSTYLEHELPER_H