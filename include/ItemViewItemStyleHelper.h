#ifndef ITEMVIEWITEMSTYLEHELPER_H
#define ITEMVIEWITEMSTYLEHELPER_H

#include <QStyleOptionViewItem>
#include <QPainter>

class ItemViewItemStyleHelper
{
public:
    void drawItemViewItemShape(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget);
    void drawText(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget);
    void drawMarket(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget);
    QSize sizeFromContents(const QStyleOptionViewItem *option, QSize contentsSize, const QWidget *widget) const;

private:
    void setupPainterForShape(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget);

private:
    const int PADDING_VERTICAL = 5;      // item签与边框之间的垂直空白量
    const int PADDING_HORIZONTAL = 12;   // item标签与边框之间的水平空白量
    const int RADIUS = 4;                // 圆角半径
    const int MARK_OFFSET_X = 3;         // 选中标记水平偏移(左侧蓝色圆角矩形)
    const int MARK_WIDTH = 3;            // 选中标记宽度
    const int MARK_RADIUS = 3;           // 选中标记圆角半径
    const qreal MARK_PADDING_TOP = 3;    // 选中标记上侧内边距
    const qreal MARK_PADDING_BOTTOM = 3; // 选中标记下侧内边距
    const int SPACING_MARK_TO_TEXT = 5;  // 选中标记到文本之间的距离
    const int SPACING_TOP = 3;           // 不用于计算绘制的到上一个item的距离(不绘制 SPACING_TOP 这段距离，视觉效果就像setSpacing(SPACING_TOP))
};

#endif // ITEMVIEWITEMSTYLEHELPER_H