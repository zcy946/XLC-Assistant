#ifndef LISTVIEWSTYLEHELPER_H
#define LISTVIEWSTYLEHELPER_H

#include <QPainter>
#include <QWidget>

class ListViewStyleHelper
{
public:
    void drawBorder(QPainter *painter, QRect rect);

private:
    const int BORDER_WIDTH = 1; // 边框宽度
    const int RADIUS = 4;       // 变宽圆角半径
};

#endif // LISTVIEWSTYLEHELPER_H