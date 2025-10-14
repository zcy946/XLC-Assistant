#include "ColorRepository.h"

#include <QApplication>
#include <QBrush>
#include <QToolTip>

static bool s_darkMode = false;

QPalette ColorRepository::standardPalette()
{
    QPalette pal;
    // TODO brush with noise.png
    pal.setColor(QPalette::Window, windowBackground());
    pal.setColor(QPalette::Base, baseBackground());
    pal.setColor(QPalette::WindowText, text());
    pal.setColor(QPalette::Text, text());

    // Text color on buttons
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

QColor ColorRepository::windowBackground()
{
    // dark blue / light gray
    return s_darkMode ? QColor("#182129") : QColor("#EFF0F1");
    // return s_darkMode ? QColor("#000000ff") : QColor("#EFF0F1");
}

QColor ColorRepository::baseBackground()
{
    // almost black / almost white
    return s_darkMode ? QColor("#0F0F0F") : QColor("#FBFBFB");
}

QColor ColorRepository::text()
{
    // gray / dark gray
    return s_darkMode ? QColor("#A5A5A5") : QColor("#252525");
}

QColor ColorRepository::pressedTextColor()
{
    // medium gray
    return QColor("#656565");
}

QColor ColorRepository::hoverTextColor()
{
    // light gray
    return QColor("#DDDDDD");
}

QColor ColorRepository::pressedOutlineColor()
{
    return QColor("#322D35");
}

QColor ColorRepository::buttonOutlineColor()
{
    return s_darkMode ? QColor("#59515F") : QColor("#9F95A3");
}

QBrush ColorRepository::hoverOutlineBrush(const QRect &rect)
{
    // Instructions from the designer:
    // "Draw line passing by center of rectangle (+4% to the right)
    // and that is perpendicular to the topleft-bottomright diagonal.
    // This line intersects the top and bottom in two points, which are the gradient stops"

    const qreal w = rect.width();
    const qreal h = rect.height();
    const qreal xmid = w * 0.54;
    const qreal xoffset = (h * h) / (2 * w); // Proportionality: xoffset / (h/2) = h / w
    const qreal x0 = xmid - xoffset;
    const qreal x1 = xmid + xoffset;

    QLinearGradient gradient(x0, h, x1, 0);
    gradient.setColorAt(0.0, QColor("#53949F"));
    gradient.setColorAt(1.0, QColor("#755579"));
    return QBrush(gradient);
}

QColor ColorRepository::buttonPressedBackground()
{
    return s_darkMode ? QColor("#171717") : QColor("#F8F7F8");
}

QColor ColorRepository::buttonHoveredBackground()
{
    QColor color = buttonPressedBackground();
    // Keep setAlphaF, as the base color is opaque
    color.setAlphaF(0.2);
    return color;
}

QColor ColorRepository::buttonBackground()
{
    // QColor(R, G, B, A) -> QColor("#AARRGGBB"). 0xa7 is the alpha component.
    return s_darkMode ? QColor("#A7211F22") : QColor("#F5F4F5") /* TODO with opacity = ? */;
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