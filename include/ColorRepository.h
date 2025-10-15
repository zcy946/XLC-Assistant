

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

    QColor windowBackground();
    QColor baseBackground();
    QColor text();

    QColor shadowColor();

    QColor disabledTextColor();
    QColor pressedTextColor();
    QColor hoverTextColor();

    QColor buttonPressedOutlineColor();
    QColor buttonHoverOutlineColor();
    QColor buttonOutlineColor();

    QColor buttonBackground();
    QColor buttonPressedBackground();
    QColor buttonHoveredBackground();
    QColor buttonDisableBackground();

    QBrush progressBarOutlineBrush(const QRect &rect);
    QBrush progressBarOutlineFadingBrush(const QRect &rect);
    QBrush progressBarContentsBrush(const QRect &rect);
    QColor progressBarTextColor(bool enabled);

    QColor listHoveredBackground();
    QColor listSelectedBackground();
    QColor listSelectedAndHoveredOutlineColor();

    QColor historyMessageListSeparator(); // 清除上下文分割线颜色
    QColor historyMessageListTimestamp(); // 时间戳的字体颜色
}

#endif // COLORREPOSITORY_H
