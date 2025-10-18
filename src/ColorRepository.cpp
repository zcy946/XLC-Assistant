#include "ColorRepository.h"

#include <QApplication>
#include <QBrush>
#include <QToolTip>

static bool s_darkMode = false;
// static QColor s_colorPrimaryNormal = "#0067C0";
static QColor s_colorPrimaryNormal = "#0078D4";

QPalette ColorRepository::standardPalette()
{
    QPalette palette;
    palette.setColor(QPalette::Window, windowBackground());
    palette.setColor(QPalette::Base, basicBackground());
    palette.setColor(QPalette::WindowText, basicText());
    palette.setColor(QPalette::Text, basicText());
    palette.setColor(QPalette::PlaceholderText, placeHolderText());

    // 按钮上的文本
    palette.setColor(QPalette::ButtonText, basicText());

    // palette.setColor(QPalette::ToolTipBase, basicBackground());
    palette.setColor(QPalette::ToolTipText, basicText());

    QToolTip::setPalette(palette);

    return palette;
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

QColor ColorRepository::basicText()
{
    return s_darkMode ? QColor("#FFFFFF") : QColor("#000000");
}

QColor ColorRepository::placeHolderText()
{
    return s_darkMode ? QColor("#BABABA") : QColor("#80000000");
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
    return s_darkMode ? QColor("#9A9A9A") : QColor("#868686");
}

QColor ColorRepository::basicFocusedHemlineColor()
{
    return primaryNormal();
}

QColor ColorRepository::basicPressedColor()
{
    return s_darkMode ? QColor("#554B4B4B") : QColor("#40CCCCCC");
}

QColor ColorRepository::basicHoveredColor()
{
    return s_darkMode ? QColor("#754B4B4B") : QColor("#70CCCCCC");
}

QColor ColorRepository::disabledTextColor()
{
    QColor colorDisabledText = basicText();
    colorDisabledText.setAlphaF(0.3);
    return colorDisabledText;
}

QColor ColorRepository::pressedTextColor()
{
    QColor colorPressedText = basicText();
    colorPressedText.setAlphaF(0.6);
    return colorPressedText;
}

QColor ColorRepository::hoverTextColor()
{
    return basicText();
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

QColor ColorRepository::listViewBorderColor()
{
    return basicBorderColor();
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
    return basicHemlineColor();
}

QColor ColorRepository::lineEditFocusedHemlineColor()
{
    return basicFocusedHemlineColor();
}

QColor ColorRepository::plainTextEditBackgroundColor()
{
    return basicBackground();
}

QColor ColorRepository::plainTextEditBorderColor()
{
    return basicBorderColor();
}

QColor ColorRepository::plainTextEditHemlineColor()
{
    return basicHemlineColor();
}

QColor ColorRepository::plainTextEditFocusedHemlineColor()
{
    return basicFocusedHemlineColor();
}

QColor ColorRepository::spinBoxBackgroundColor()
{
    return basicBackground();
}

QColor ColorRepository::spinBoxBorderColor()
{
    return basicBorderColor();
}

QColor ColorRepository::spinBoxArrowColor()
{
    return spinBoxBackgroundColor();
}

QColor ColorRepository::spinBoxPressedArrowColor()
{
    return basicPressedColor();
}

QColor ColorRepository::spinBoxHoveredArrowColor()
{
    return basicHoveredColor();
}

QColor ColorRepository::spinBoxHemlineColor()
{
    return basicHemlineColor();
}

QColor ColorRepository::spinBoxFocusedHemlineColor()
{
    return basicFocusedHemlineColor();
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