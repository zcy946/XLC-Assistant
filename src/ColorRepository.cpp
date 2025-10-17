#include "ColorRepository.h"

#include <QApplication>
#include <QBrush>
#include <QToolTip>

static bool s_darkMode = false;
static QColor s_colorPrimaryNormal = "#0067C0";

QPalette ColorRepository::standardPalette()
{
    QPalette pal;
    // TODO brush with noise.png
    pal.setColor(QPalette::Window, windowBackground());
    pal.setColor(QPalette::Base, basicBackground());
    pal.setColor(QPalette::WindowText, text());
    pal.setColor(QPalette::Text, text());

    // 按钮上的文本
    pal.setColor(QPalette::ButtonText, text());

    // pal.setColor(QPalette::ToolTipBase, basicBackground());
    pal.setColor(QPalette::ToolTipText, text());

    QToolTip::setPalette(pal);

    return pal;
}

void ColorRepository::setDarkMode(bool dark)
{
    s_darkMode = dark;
    qApp->setPalette(standardPalette());
}

QColor ColorRepository::primaryNormal()
{
    return s_colorPrimaryNormal;
}

QColor ColorRepository::windowBackground()
{
    return s_darkMode ? QColor("#202020") : QColor("#F3F3F3");
}

QColor ColorRepository::basicBackground()
{
    // return s_darkMode ? QColor("#292929") : QColor("#FDFDFD");
    return s_darkMode ? QColor("#292929") : QColor("#FCFCFC");
}

QColor ColorRepository::text()
{
    return s_darkMode ? QColor("#FFFFFF") : QColor("#000000");
}

QColor ColorRepository::shadowColor()
{
    QColor colorShadow = s_darkMode ? QColor("#9C9B9E") : QColor("#D1D1D1");
    // QColor colorShadow = s_darkMode ? QColor("#9C9B9E") : QColor("#707070");
    colorShadow.setAlpha(255 - 2);
    return colorShadow;
}

QColor ColorRepository::basicBorderColor()
{
    return s_darkMode ? QColor("#4B4B4B") : QColor("#E5E5E5");
}

QColor ColorRepository::basicHemlineColor()
{
    return primaryNormal();
}

QColor ColorRepository::disabledTextColor()
{
    QColor colorDisabledText = text();
    colorDisabledText.setAlphaF(0.3);
    return colorDisabledText;
}

QColor ColorRepository::pressedTextColor()
{
    QColor colorPressedText = text();
    colorPressedText.setAlphaF(0.6);
    return colorPressedText;
}

QColor ColorRepository::hoverTextColor()
{
    return text();
}

QColor ColorRepository::buttonBorderColor()
{
    return s_darkMode ? QColor("#4B4B4B") : QColor("#D1D1D1");
}

QColor ColorRepository::buttonPressedBorderColor()
{
    return QColor("#B3B3B3");
}
QColor ColorRepository::buttonHoverOutlineColor()
{
    return QColor("#C7C7C7");
}

QColor ColorRepository::buttonBackgroundColor()
{
    return s_darkMode ? QColor("#A7211F22") : QColor("#FDFDFD");
}

QColor ColorRepository::buttonPressedBackgroundColor()
{
    return s_darkMode ? QColor("#171717") : QColor("#F3F3F3");
}

QColor ColorRepository::buttonHoveredBackgroundColor()
{
    QColor color = buttonPressedBackgroundColor();
    return color;
}

QColor ColorRepository::buttonDisableBackgroundColor()
{
    return s_darkMode ? QColor("#A7211F22") : QColor("#F5F5F5");
}

QColor ColorRepository::itemViewItemMarkColor()
{
    return primaryNormal();
}

QColor ColorRepository::itemViewItemBackgroundColor()
{
    return basicBackground();
}

QColor ColorRepository::itemViewItemSelectedBackgroundColor()
{
    return s_darkMode ? QColor("#333333") : QColor("#EFEFEF");
}

QColor ColorRepository::itemViewItemHoveredBackgroundColor()
{
    return itemViewItemSelectedBackgroundColor();
}

QColor ColorRepository::itemViewItemSelectedAndHoveredBackgroundColor()
{
    return s_darkMode ? QColor("#303030") : QColor("#F4F4F4");
}

QColor ColorRepository::scrollBarBackgroundColor()
{
    return basicBackground();
}

QColor ColorRepository::scrollBarSliderColor()
{
    return s_darkMode ? QColor("#9F9F9F") : QColor("#A0A0A0");
}

QColor ColorRepository::scrollBarSliderHoveredColor()
{
    return scrollBarSliderColor();
}

QColor ColorRepository::scrollBarSliderSelectedColor()
{
    return scrollBarSliderColor();
}

QColor ColorRepository::scrollBarArrowColor()
{
    return scrollBarSliderColor();
}

QColor ColorRepository::lineEditBackgroundColor()
{
    return basicBackground();
}

QColor ColorRepository::lineEditHoveredBackgroundColor()
{
    return basicBackground();
}

QColor ColorRepository::lineEditFocusedBackgroundColor()
{
    return basicBackground();
}

QColor ColorRepository::lineEditBorderColor()
{
    return basicBorderColor();
}

QColor ColorRepository::lineEditHemlineColor()
{
    return s_darkMode ? QColor("#9A9A9A") : QColor("#868686");
}

QColor ColorRepository::lineEditFocusedHemlineColor()
{
    return basicHemlineColor();
}

QColor ColorRepository::listHoveredBackgroundColor()
{
    return QColor("#E5F3FF");
}

QColor ColorRepository::listSelectedBackgroundColor()
{
    return QColor("#CCE8FF");
}

QColor ColorRepository::listSelectedAndHoveredOutlineColor()
{
    return QColor("#99D1FF");
}

QColor ColorRepository::historyMessageListSeparator()
{
    return QColor("#D0D0D0");
}

QColor ColorRepository::historyMessageListTimestamp()
{
    return QColor("#9E9E9E");
}