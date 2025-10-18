#ifndef PLAINTEXTEDITSTYLEHELPER_H
#define PLAINTEXTEDITSTYLEHELPER_H

#include <QStyleOptionFrame>
#include <QPainter>

class PlainTextEditStyleHelper
{
public:
    void drawBackgroundAndBorder(const QStyleOptionFrame *option, QPainter *painter, const QWidget *widget) const;
    void drawHemline(const QStyleOptionFrame *option, QPainter *painter, const QWidget *widget) const;
    QRect contentArea(const QStyleOptionFrame *option, const QWidget *widget) const;

private:
    const int RADIUS = 6;                 // 圆角半径
    const int BORDER_WIDTH = 1;           // 边框宽度
    const int MARGIN_HEMLINE = 4;         // 底部边缘矩形左下角水平外边距
    const int MARGIN_HEMLINE_FOCUSED = 6; // 底部边缘矩形左下角水平聚焦时外边距
    const int PADDING_HORIZONTAL = 12;    // 文本与边框之间的水平间距
    const int PADDING_VERTICAL = 6;       // 文本与边框之间的垂直
};

#endif // PLAINTEXTEDITSTYLEHELPER_H