#ifndef LINEEDITSTYLEHELPER_H
#define LINEEDITSTYLEHELPER_H

#include <QStyleOptionFrame>
#include <QPainter>
#include <QWidget>

class LineEditStyleHelper
{
public:
    void drawLineEditShape(const QStyleOptionFrame *optionLineEdit, QPainter *painter, const QWidget *widget);

private:
    void drawBackground(const QStyleOptionFrame *optionLineEdit, QPainter *painter);
    void drawBorder(const QStyleOptionFrame *optionLineEdit, QPainter *painter);
    void drawHemline(const QStyleOptionFrame *optionLineEdit, QPainter *painter);

private:
    const int RADIUS = 6;         // 圆角半径
    const int BORDER_WIDTH = 1;   // 边框宽度
    const int MARGIN_HEMLINE = 6; // 底部边缘水平外边距
};

#endif // LINEEDITSTYLEHELPER_H