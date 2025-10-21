#ifndef MENUSTYLEHELPER_H
#define MENUSTYLEHELPER_H

#include <QStyleOptionMenuItem>
#include <QPainter>

class MenuStyleHelper
{
public:
    // 绘制背景
    void drawBackground(const QStyleOptionMenuItem *option, QPainter *painter, const QWidget *widget) const;
    // 绘制阴影
    void drawShadow(const QStyleOptionMenuItem *option, QPainter *painter, const QWidget *widget) const;
    // 绘制item背景
    void drawBackgroundItem(const QStyleOptionMenuItem *option, QPainter *painter, const QWidget *widget) const;
    // 绘制分割线
    void drawSeparator(const QStyleOptionMenuItem *option, QPainter *painter, const QWidget *widget) const;
    // 绘制选中标记
    void drawCheckIndicator(const QStyleOptionMenuItem *option, QPainter *painter, const QWidget *widget) const;
    // 绘制文本和快捷方式
    void drawText(const QStyleOptionMenuItem *option, QPainter *painter, const QWidget *widget) const;
    // 绘制图标
    void drawIcon(int sizeIconOriginal, const QStyleOptionMenuItem *option, QPainter *painter, const QWidget *widget) const;
    // 绘制展开图标
    void drawIconExpand(const QStyleOptionMenuItem *option, QPainter *painter, const QWidget *widget) const;
    // item大小
    QSize sizeMenuItem(const QStyleOptionMenuItem *option, QSize contentsSize, const QWidget *widget) const;

private:
    const int WIDTH_SHADOW = 6;                      // 阴影宽度
    const int WIDTH_BORDER = 1;                      // 边框宽度
    const int RADIUS = 5;                            // 圆角半径
    const int RADIUS_ITEM = 4;                       // item圆角半径
    const qreal HEIGHT_SEPARATOR = 1.5;              // 分割线圆角半径
    const int MARGIN_HORIZONTAL_ITEM = 1;            // item水平外边距
    const int PADDING_ITEM = 5;                      // item内边距
    const QString ICONFONT_NAME = "ElaAwesome";      // 字体图标名称
    const int ICONFONT_SIZE = 18;                    // 字体图标大小
    const QChar ICONFONT_Check = QChar(0xea6c);      // 字体图标-对勾
    const QChar ICONFONT_None = QChar(0x0000);       // 字体图标-无
    const QChar ICONFONT_AngleRight = QChar(0xe833); // 字体图标-扩展
    const int WIDTH_MIN = 100;                       // 最小宽度
};

#endif // MENUSTYLEHELPER_H