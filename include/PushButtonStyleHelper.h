#ifndef PUSHBUTTONSTYLEHELPER_H
#define PUSHBUTTONSTYLEHELPER_H

#include <QPainter>
#include <QStyleOptionButton>
#include <QSize>
#include <QPushButton>

class PushButtonStyleHelper
{
public:
    void drawButtonShape(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget) const;
    void drawText(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget) const;
    void drawShadow(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget) const;
    QSize sizeFromContents(const QStyleOptionButton *option, QSize contentsSize, const QWidget *widget) const;

    int padding();

private:
    void setupPainterForShape(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget) const;

private:
    const int PADDING_VERTICAL = 5;    // 按钮标签与边框之间的垂直空白量
    const int PADDING_HORIZONTAL = 12; // 按钮标签与边框之间的水平空白量
    const int RADIUS = 4;              // 圆角半径
    const int WIDTH_BORDER = 1;        // 边框宽度
    const int WIDTH_MIN = 96;          // 最小宽度
    const int RADIUS_SHADOW = 3;       // 阴影模糊半径
    const int OFFSET_X_SHADOW = 0;     // 阴影 x 偏移
    const int OFFSET_Y_SHADOW = 0;     // 阴影 y 偏移
    const int SIZE_FONT = 14;          // 文本字体大小
};

#endif // PUSHBUTTONSTYLEHELPER_H