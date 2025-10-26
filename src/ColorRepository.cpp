#include "ColorRepository.h"
#include <QApplication>
#include <QToolTip>

QPalette ColorRepository::standardPalette()
{
    QPalette palette;
    palette.setColor(QPalette::Window, windowBackgroundColor());
    palette.setColor(QPalette::Base, baseBackgroundColor());
    palette.setColor(QPalette::WindowText, basicTextColor());
    palette.setColor(QPalette::Text, basicTextColor());
    palette.setColor(QPalette::PlaceholderText, placeHolderText());
    palette.setColor(QPalette::ButtonText, basicTextColor());
    palette.setColor(QPalette::ToolTipBase, baseBackgroundColor());
    palette.setColor(QPalette::ToolTipText, basicTextColor());
    QToolTip::setPalette(palette);
    return palette;
}

void ColorRepository::setDarkMode(bool dark)
{
    if (s_darkMode == dark)
        return;
    s_darkMode = dark;
    qApp->setPalette(standardPalette());
}

void ColorRepository::setPrimaryColor(const QColor &colorNew)
{
    if (!colorNew.isValid())
        return;
    s_colorPrimaryNormal = colorNew;
    qApp->setPalette(standardPalette());
}

QColor ColorRepository::primaryNormal()
{
    return s_colorPrimaryNormal;
}

QColor ColorRepository::primaryPressed()
{
    // return s_darkMode ? QColor("#42A1D2") : QColor("#3183CA");
    return primaryNormal().darker(105);
}

QColor ColorRepository::primaryHovered()
{
    // return s_darkMode ? QColor("#47B1E8") : QColor("#1975C5");
    return primaryNormal().darker(108);
}

QColor ColorRepository::windowBackgroundColor()
{
    return s_darkMode ? QColor("#202020") : QColor("#F3F3F3");
}

QColor ColorRepository::baseBackgroundColor()
{
    // return s_darkMode ? QColor("#292929") : QColor("#FDFDFD");
    return s_darkMode ? QColor("#292929") : QColor("#FCFCFC");
}

QColor ColorRepository::dialogBaseBackground()
{
    return s_darkMode ? QColor("#1F1F1F") : QColor("#FFFFFF");
}

QColor ColorRepository::basicTextColor()
{
    return s_darkMode ? s_colorTextDrakMode : s_colorTextLightMode;
}

QColor ColorRepository::basicDisableText()
{
    return s_darkMode ? QColor("#A7A7A7") : QColor("#B6B6B6");
}

QColor ColorRepository::placeHolderText()
{
    return s_darkMode ? QColor("#BABABA") : QColor("#80000000");
}

QColor ColorRepository::shadowColor()
{
    // QColor colorShadow = s_darkMode ? QColor("#9C9B9E") : QColor("#D1D1D1");
    QColor colorShadow = s_darkMode ? QColor("#9C9B9E") : QColor("#707070");
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
    return s_darkMode ? QColor("#3A3A3A") : QColor("#F7F7F7");
}

QColor ColorRepository::basicHoveredColor()
{
    return s_darkMode ? QColor("#404040") : QColor("#F3F3F3");
}

QColor ColorRepository::basicDisabledColor()
{
    return s_darkMode ? QColor("#2A2A2A") : QColor("#F5F5F5");
}

QColor ColorRepository::basicPressedAlphaColor()
{
    return s_darkMode ? QColor(0x4B, 0x4B, 0x4B, 55) : QColor(0xCC, 0xCC, 0xCC, 40);
}

QColor ColorRepository::basicHoveredAlphaColor()
{
    return s_darkMode ? QColor(0x4B, 0x4B, 0x4B, 75) : QColor(0xCC, 0xCC, 0xCC, 70);
}

QColor ColorRepository::basicSelectedAlphaColor()
{
    return s_darkMode ? QColor(0x4B, 0x4B, 0x4B, 75) : QColor(0xCC, 0xCC, 0xCC, 70);
}

QColor ColorRepository::basicSelectedAndHoveredAlphaColor()
{
    return s_darkMode ? QColor(0x4B, 0x4B, 0x4B, 55) : QColor(0xCC, 0xCC, 0xCC, 40);
}

QColor ColorRepository::disabledTextColor()
{
    QColor colorDisabledText = basicTextColor();
    colorDisabledText.setAlphaF(0.3);
    return colorDisabledText;
}

QColor ColorRepository::pressedTextColor()
{
    QColor colorPressedText = basicTextColor();
    colorPressedText.setAlphaF(0.6);
    return colorPressedText;
}

QColor ColorRepository::hoverTextColor()
{
    return basicTextColor();
}

QColor ColorRepository::buttonBorderColor()
{
    return basicBorderColor();
}

QColor ColorRepository::buttonPressedBorderColor()
{
    return buttonBorderColor();
}

QColor ColorRepository::buttonHoverOutlineColor()
{
    return buttonBorderColor();
}

QColor ColorRepository::buttonBackgroundColor()
{
    // return s_darkMode ? QColor("#A7211F22") : QColor("#FDFDFD");
    return baseBackgroundColor();
}

QColor ColorRepository::buttonPressedBackgroundColor()
{
    // return s_darkMode ? QColor("#171717") : QColor("#F3F3F3");
    return basicPressedColor();
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

QColor ColorRepository::buttonHemlineColor()
{
    return s_darkMode ? QColor("#454545") : QColor("#D1D1D1");
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
    return baseBackgroundColor();
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
    return baseBackgroundColor();
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
    return baseBackgroundColor();
}

QColor ColorRepository::lineEditHoveredBackgroundColor()
{
    return baseBackgroundColor();
}

QColor ColorRepository::lineEditFocusedBackgroundColor()
{
    return baseBackgroundColor();
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
    return baseBackgroundColor();
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
    return baseBackgroundColor();
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
    return basicPressedAlphaColor();
}

QColor ColorRepository::spinBoxHoveredArrowColor()
{
    return basicHoveredAlphaColor();
}

QColor ColorRepository::spinBoxHemlineColor()
{
    return basicHemlineColor();
}

QColor ColorRepository::spinBoxFocusedHemlineColor()
{
    return basicFocusedHemlineColor();
}

QColor ColorRepository::checkBoxBackgroundColor(bool status)
{
    if (status)
        return primaryNormal();
    else
        return baseBackgroundColor();
}

QColor ColorRepository::checkBoxPressedBackgroundColor(bool status)
{
    if (status)
        return primaryPressed();
    else
        return basicPressedColor();
}

QColor ColorRepository::checkBoxHoveredBackgroundColor(bool status)
{
    if (status)
        return primaryHovered();
    else
        return basicHoveredColor();
}

QColor ColorRepository::checkBoxBorderColor()
{
    return basicBorderColor();
}

QColor ColorRepository::checkBoxIndicatorColor()
{
    return s_darkMode ? s_colorTextLightMode : s_colorTextDrakMode;
}

QColor ColorRepository::comboBoxBackgroundColor()
{
    return baseBackgroundColor();
}

QColor ColorRepository::comboBoxDisabledBackgroundColor()
{
    return basicDisabledColor();
}

QColor ColorRepository::comboBoxHoveredBackgroundColor()
{
    return basicHoveredColor();
}

QColor ColorRepository::comboBoxEditedBackgroundColor()
{
    return dialogBaseBackground();
}

QColor ColorRepository::comboBoxBorderColor()
{
    return basicBorderColor();
}

QColor ColorRepository::comboBoxHemlineColor()
{
    return basicHemlineColor();
}

QColor ColorRepository::comboBoxFocusedHemlineColor()
{
    return basicFocusedHemlineColor();
}

QColor ColorRepository::popupBorderColor()
{
    return s_darkMode ? QColor("#474747") : QColor("#D6D6D6");
}

QColor ColorRepository::popupBackgroundColor()
{
    return s_darkMode ? QColor("#2C2C2C") : QColor("#FAFAFA");
}

QColor ColorRepository::menuSeparatorColor()
{
    return basicBorderColor();
}

QColor ColorRepository::menuBackgroundColor()
{
    return s_darkMode ? QColor("#2C2C2C") : QColor("#FAFAFA");
}

QColor ColorRepository::menuHoveredBackgroundColor()
{
    return s_darkMode ? QColor("#383838") : QColor("#F0F0F0");
}

QColor ColorRepository::menuBorderColor()
{
    return s_darkMode ? QColor("#474747") : QColor("#D6D6D6");
}

QColor ColorRepository::listHoveredBackgroundColor()
{
    return itemViewItemHoveredBackgroundColor();
}

QColor ColorRepository::listSelectedBackgroundColor()
{
    return itemViewItemSelectedBackgroundColor();
}

QColor ColorRepository::listSelectedAndHoveredOutlineColor()
{
    return itemViewItemSelectedAndHoveredBackgroundColor();
}

QColor ColorRepository::groupBoxBackgroundColor()
{
    return s_darkMode ? QColor("#272727") : QColor("#F7F7F7");
}

QColor ColorRepository::groupBoxBorderColor()
{
    return basicBorderColor();
}

QColor ColorRepository::historyMessageListSeparator()
{
    return QColor("#D0D0D0");
}

QColor ColorRepository::historyMessageListTimestamp()
{
    return QColor("#9E9E9E");
}

QColor ColorRepository::navigationItemBackgroundColor()
{
    return windowBackgroundColor();
}

QColor ColorRepository::navigationPressedBackgroundColor()
{
    return basicSelectedAndHoveredAlphaColor();
}

QColor ColorRepository::navigationItemHoveredBackgroundColor()
{
    return basicHoveredAlphaColor();
}

QColor ColorRepository::navigationItemSelectedBackgroundColor()
{
    return basicSelectedAlphaColor();
}

QColor ColorRepository::navigationSelectedAndPressedBackgroundColor()
{
    return basicHoveredAlphaColor();
}

QColor ColorRepository::navigationItemSelectedAndHoveredBackgroundColor()
{
    return basicSelectedAndHoveredAlphaColor();
}
