#ifndef ITEMVIEWITEMSTYLEHELPER_H
#define ITEMVIEWITEMSTYLEHELPER_H

#include <QStyleOptionViewItem>
#include <QPainter>

class XlcStyle;
class ItemViewItemStyleHelper
{
public:
    // 绘制背景
    void drawBackground(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget);
    // 绘制文本
    void drawText(const XlcStyle *style, const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget) const;
    // 绘制选中标记
    void drawMarket(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget) const;
    // 绘制复选框
    void drawCheckIndicator(const XlcStyle *style, const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget) const;
    // 绘制展开指示器(QTreeView)
    void drawBranchIndicator(const XlcStyle *style, const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget) const;
    // 全部内容的区域
    QSize sizeFromContents(const QStyleOptionViewItem *option, QSize sizeOriginal, const QWidget *widget) const;
    // 文本区域
    QRect rectText(const QStyleOptionViewItem *option, const QWidget *widget, QRect rectTextOriginal) const;
    // 复选框区域
    QRect rectCheckIndicator(const QStyleOptionViewItem *option, const QWidget *widget) const;
    // 复选框响应点击区域
    QRect rectClickCheckIndicator(const XlcStyle *style, const QStyleOptionViewItem *option, const QWidget *widget);
    // 展开指示器区域
    QRect rectBranchIndicator(const QStyleOptionViewItem *option, const QWidget *widget, QRect rectBranchIndicatorOriginal);

private:
    void setupPainterForShape(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget);

private:
    const int PADDING_VERTICAL = 5;                  // item签与边框之间的垂直空白量
    const int PADDING_HORIZONTAL = 12;               // item标签与边框之间的水平空白量
    const int RADIUS = 4;                            // 圆角半径
    const int OFFSET_MARK_X = 3;                     // 选中标记水平偏移(左侧蓝色圆角矩形)
    const int WIDTH_MARK = 3;                        // 选中标记宽度
    const int RADIUS_MARK = 3;                       // 选中标记圆角半径
    const qreal PADDING_MARK_TOP = 7;                // 选中标记上侧内边距
    const qreal PADDING_MARK_BOTTOM = 7;             // 选中标记下侧内边距
    const int SPACING_MARK_TO_TEXT = 5;              // 选中标记到文本之间的距离
    const int SPACING_TOP = 2;                       // 不用于计算绘制的到上一个item的距离(不绘制 SPACING_TOP 这段距离，视觉效果就像setSpacing(SPACING_TOP))
    const int WIDTH_CHECK_INDICATOR = 16;            // 复选框宽
    const int HEIGHT_CHECK_INDICATOR = 16;           // 复选框高
    const QString ICONFONT_NAME = "ElaAwesome";      // 字体图标名称
    const QChar ICONFONT_AngleDown = QChar(0xe832);  // 字体图标 - 下箭头
    const QChar ICONFONT_AngleRight = QChar(0xe833); // 字体图标 - 右箭头
};

#endif // ITEMVIEWITEMSTYLEHELPER_H