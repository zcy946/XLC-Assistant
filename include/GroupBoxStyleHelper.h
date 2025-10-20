#ifndef GROPBOXSTYLEHELPER_H
#define GROPBOXSTYLEHELPER_H

#include <QStyleOptionGroupBox>
#include <QPainter>

class XlcStyle;
class GroupBoxStyleHelper
{
public:
    // 绘制边框
    void drawBorder(const XlcStyle *style, const QStyleOptionGroupBox *option, QPainter *painter, const QWidget *widget) const;
    // 绘制文本
    void drawLabel(const XlcStyle *style, const QStyleOptionGroupBox *option, QPainter *painter, const QWidget *widget) const;
    // 边框区域
    QRect rectFrame(const QStyleOptionGroupBox *option, const QWidget *widget) const;
    // 文本标签区域
    QRect rectLabel(const QStyleOptionGroupBox *option, const QWidget *widget, QRect rectLabelOriginal) const;
    // 内部组件区域
    QRect rectContents(const QStyleOptionGroupBox *option, const QWidget *widget, QRect rectContentsOriginal) const;

private:
    const int WIDTH_BORDER = 2;  // 边框宽度
    const int RADIUS = 6;        // 圆角半径
    const int MARGIN_LABEL = 12; // 文本标签左侧到边框的距离
    const int PADDING_LABEL = 2; // 文本与边框线之间的距离
};

#endif // GROPBOXSTYLEHELPER_H