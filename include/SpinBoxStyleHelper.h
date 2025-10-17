#ifndef SPINBOXSTYLEHELPER_H
#define SPINBOXSTYLEHELPER_H

#include <QStyleOptionSpinBox>
#include <QPainter>
#include <QWidget>

class SpinBoxStyleHelper
{
public:
    void drawBackground(const QStyleOptionSpinBox *option, QPainter *painter, const QWidget *widget);
    void drawSubControls(const QStyleOptionSpinBox *option, QPainter *painter, const QWidget *widget, const QRect &rectSubLine, const QRect &rectAddLine);
    void drawHemline(const QStyleOptionSpinBox *option, QPainter *painter, const QWidget *widget);

private:
    const int BORDER_WIDTH = 1;                     // 边框宽度
    const int RADIUS = 4;                           // 圆角半径
    const int MARGIN_HEMLINE = 4;                   // 底部边缘水平外边距
    const QString ICONFONT_NAME = "ElaAwesome";     // 字体图标名称
    const int ICONFONT_SIZE = 14;                   // 字体图标大小
    const QChar ICONFONT_AngleUp = QChar(0xe839);   // 上箭头图标
    const QChar ICONFONT_AngleDown = QChar(0xe832); // 下箭头图标
};

#endif // SPINBOXSTYLEHELPER_H