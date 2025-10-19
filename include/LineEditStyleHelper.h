#ifndef LINEEDITSTYLEHELPER_H
#define LINEEDITSTYLEHELPER_H

#include <QStyleOptionFrame>
#include <QPainter>
#include <QWidget>

class LineEditStyleHelper
{
public:
    void drawLineEditShape(const QStyleOptionFrame *optionLineEdit, QPainter *painter, const QWidget *widget);
    // 文本区域
    QRect rectText(const QStyleOptionFrame *option, const QWidget *widget, QRect rectTextOriginal);
    // 整个控件区域
    QSize rectAll(const QStyleOptionFrame *option, const QWidget *widget, QSize sizeAllOriginal);

private:
    void drawBackground(const QStyleOptionFrame *optionLineEdit, QPainter *painter);
    void drawBorder(const QStyleOptionFrame *optionLineEdit, QPainter *painter);
    void drawHemline(const QStyleOptionFrame *optionLineEdit, QPainter *painter);

private:
    const int RADIUS = 6;                 // 圆角半径
    const int WIDTH_BORDER = 1;           // 边框宽度
    const int MARGIN_HEMLINE = 4;         // 底部边缘矩形左下角水平外边距
    const int MARGIN_HEMLINE_FOCUSED = 6; // 底部边缘矩形左下角水平聚焦时外边距
    const int PADDING_VERTICAL = 6;       // 标签与边框之间的垂直空白量
    const int PADDING_HORIZONTAL = 12;    // 标签与边框之间的水平空白量
};

#endif // LINEEDITSTYLEHELPER_H