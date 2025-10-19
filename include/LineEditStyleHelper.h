#ifndef LINEEDITSTYLEHELPER_H
#define LINEEDITSTYLEHELPER_H

#include <QStyleOptionFrame>
#include <QPainter>
#include <QWidget>

class LineEditStyleHelper
{
public:
    void drawLineEditShape(const QStyleOptionFrame *optionLineEdit, QPainter *painter, const QWidget *widget);
    QRect subElementRect(QStyle::SubElement subElement, const QStyleOptionFrame *option, const QWidget *widget, const QRect &rectBasic);
    QSize sizeFromContents(const QStyleOptionFrame *option, QSize sizeBasic, const QWidget *widget);

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