#ifndef SCROLLBARSTYLEHELPER_H
#define SCROLLBARSTYLEHELPER_H

#include <QPainter>
#include <QStyleOptionSlider>

class ScrollBarStyleHelper
{
public:
    void drawBackground(const QStyleOptionSlider *option, QPainter *painter, const QWidget *widget);
    void drawGroove(const QStyleOptionSlider *option, QPainter *painter, const QWidget *widget, const QRect &grooveRect);
    void drawSlider(const QStyleOptionSlider *option, QPainter *painter, const QWidget *widget, const QRect &rectSlider);
    void drawSubControls(const QStyleOptionSlider *option, QPainter *painter, const QWidget *widget, const QRect &rectSubLine, const QRect &rectAddLine);
    int scrollBarExtent() const;

private:
    const int RADIUS = 6;                  // 背景圆角半径
    const int PADDING = 3;                 // scrollbar与贴合面的距离
    const int SCROLLBAR_EXTENT = 6;        // scrollbar范围
    const int SIDE_LENGTH = 8;             // 箭头几何常量(用来辅助计算三角形的顶点位置)
    const int SPACING_SLIDER_TO_ARROW = 3; // 滑块和行增/减按钮(箭头)之间的距离
    const qreal PI = 3.14;
};

#endif // SCROLLBARSTYLEHELPER_H