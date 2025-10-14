#ifndef PAINTERHELPER_H
#define PAINTERHELPER_H

#include <QPainter>
#include <QStyleOptionViewItem>

class PainterHelper
{
public:
    static void drawBackground(QPainter *painter, const QStyleOptionViewItem &option, QRect rectBackground, int radius, const QColor &colorNormalBackground = QColor());

private:
    static const int OUTLINE_WIDTH = 2; // item 轮廓宽度
};

#endif // PAINTERHELPER_H