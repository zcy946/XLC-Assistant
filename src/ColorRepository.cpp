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
    pal.setColor(QPalette::Base, baseBackground());
    pal.setColor(QPalette::WindowText, text());
    pal.setColor(QPalette::Text, text());

    // 按钮上的文本
    pal.setColor(QPalette::ButtonText, text());

    // pal.setColor(QPalette::ToolTipBase, baseBackground());
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

QColor ColorRepository::baseBackground()
{
    return s_darkMode ? QColor("#292929") : QColor("#FDFDFD");
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

QColor ColorRepository::buttonOutlineColor()
{
    return s_darkMode ? QColor("#4B4B4B") : QColor("#D1D1D1");
}

QColor ColorRepository::buttonPressedOutlineColor()
{
    return QColor("#B3B3B3");
}
QColor ColorRepository::buttonHoverOutlineColor()
{
    return QColor("#C7C7C7");
}

QColor ColorRepository::buttonBackground()
{
    return s_darkMode ? QColor("#A7211F22") : QColor("#FDFDFD");
}

QColor ColorRepository::buttonPressedBackground()
{
    return s_darkMode ? QColor("#171717") : QColor("#F3F3F3");
}

QColor ColorRepository::buttonHoveredBackground()
{
    QColor color = buttonPressedBackground();
    return color;
}

QColor ColorRepository::buttonDisableBackground()
{
    return s_darkMode ? QColor("#A7211F22") : QColor("#F5F5F5");
}

QColor ColorRepository::itemViewItemMarkColor()
{
    return primaryNormal();
}

QColor ColorRepository::itemViewItemBackgroundColor()
{
    return s_darkMode ? QColor("#292929") : QColor("#FFFFFF");
}

QColor ColorRepository::itemViewItemSelectedBackgroundColor()
{
    return s_darkMode ? QColor("#3B3B3B") : QColor("#F1F1F1");
}

QColor ColorRepository::itemViewItemHoveredBackgroundColor()
{
    return itemViewItemSelectedBackgroundColor();
}

QColor ColorRepository::itemViewItemSelectedAndHoveredBackgroundColor()
{
    return s_darkMode ? QColor("#393939") : QColor("#F5F5F5");
}

QColor ColorRepository::scrollBarBackgroundColor()
{
    return baseBackground();
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

QBrush ColorRepository::progressBarOutlineBrush(const QRect &rect)
{
    QLinearGradient gradient(0, rect.height(), rect.width(), 0);
    gradient.setColorAt(0.0, QColor("#11C2E1"));
    gradient.setColorAt(1.0, QColor("#893A94"));
    return QBrush(gradient);
}

QBrush ColorRepository::progressBarOutlineFadingBrush(const QRect &rect)
{
    QLinearGradient gradient(0, rect.height(), rect.width(), 0);
    gradient.setColorAt(0.0, QColor("#11C2E1"));
    gradient.setColorAt(1.0, QColor("#893A94"));
    return QBrush(gradient);
}

QBrush ColorRepository::progressBarContentsBrush(const QRect &rect)
{
    // same as outline brush but with 37% opacity (0x60 in hex)
    // QColor(R, G, B, A) -> QColor("#AARRGGBB"). 0x60 is the alpha component.
    QLinearGradient gradient(0, rect.height(), rect.width(), 0);
    gradient.setColorAt(0.0, QColor("#6011C2E1"));
    gradient.setColorAt(1.0, QColor("#60893A94"));
    return QBrush(gradient);
}

QColor ColorRepository::progressBarTextColor(bool enabled)
{
    QColor textColor = text();
    // Keep setAlphaF, as the base color is opaque
    if (!enabled)
        textColor.setAlphaF(textColor.alphaF() / 2.0);
    return textColor;
}

QColor ColorRepository::listHoveredBackground()
{
    return QColor("#E5F3FF");
}

QColor ColorRepository::listSelectedBackground()
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