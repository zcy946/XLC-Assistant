

#ifndef COLORREPOSITORY_H
#define COLORREPOSITORY_H

#include <QColor>
#include <QPalette>

/**
 * Our colors. They are separate from the widget style so that custom widgets can also use them directly.
 */
namespace ColorRepository
{
    QPalette standardPalette();
    void setDarkMode(bool dark);

    // 主颜色
    QColor primaryNormal();

    // 窗口背景颜色(例如: QWidget)
    QColor windowBackground();
    // 通用控件背景颜色(例如: QListView)
    QColor basicBackground();
    // 通用文本颜色
    QColor text();

    // 阴影颜色
    QColor shadowColor();

    // 通用边框颜色
    QColor basicBorderColor();
    // 通用底边缘颜色
    QColor basicHemlineColor();

    QColor disabledTextColor();
    QColor pressedTextColor();
    QColor hoverTextColor();

    QColor buttonBorderColor();
    QColor buttonPressedBorderColor();
    QColor buttonHoverOutlineColor();
    QColor buttonBackgroundColor();
    QColor buttonPressedBackgroundColor();
    QColor buttonHoveredBackgroundColor();
    QColor buttonDisableBackgroundColor();

    // listview中item左侧缘圆角矩形颜色
    QColor itemViewItemMarkColor();
    QColor itemViewItemBackgroundColor();
    QColor itemViewItemSelectedBackgroundColor();
    QColor itemViewItemHoveredBackgroundColor();
    QColor itemViewItemSelectedAndHoveredBackgroundColor();

    QColor scrollBarBackgroundColor();
    QColor scrollBarSliderColor();
    QColor scrollBarSliderHoveredColor();
    QColor scrollBarSliderSelectedColor();
    QColor scrollBarArrowColor();

    QColor lineEditBackgroundColor();
    QColor lineEditHoveredBackgroundColor();
    QColor lineEditFocusedBackgroundColor();
    QColor lineEditBorderColor();
    QColor lineEditHemlineColor();
    QColor lineEditFocusedHemlineColor();

    QColor listHoveredBackgroundColor();
    QColor listSelectedBackgroundColor();
    QColor listSelectedAndHoveredOutlineColor();

    QColor historyMessageListSeparator(); // 清除上下文分割线颜色
    QColor historyMessageListTimestamp(); // 时间戳的字体颜色
}

#endif // COLORREPOSITORY_H
