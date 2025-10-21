#include "MenuStyleHelper.h"
#include "ColorRepository.h"
#include <QPainterPath>
#include <QDebug>

void MenuStyleHelper::drawBackground(const QStyleOptionMenuItem *option, QPainter *painter, const QWidget *widget) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(ColorRepository::menuBorderColor(), WIDTH_BORDER));
    painter->setBrush(ColorRepository::baseBackgroundColor());
    painter->drawRect(option->rect.adjusted(WIDTH_BORDER,
                                                   WIDTH_BORDER,
                                                   WIDTH_BORDER,
                                                   WIDTH_BORDER));
    painter->restore();
}

void MenuStyleHelper::drawShadow(const QStyleOptionMenuItem *option, QPainter *painter, const QWidget *widget) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    QColor color = ColorRepository::shadowColor();
    for (int i = 0; i < WIDTH_SHADOW; i++)
    {
        path.addRoundedRect(option->rect.x() + WIDTH_SHADOW - i,
                            option->rect.y(),
                            option->rect.width() - (WIDTH_SHADOW - i) * 2,
                            option->rect.height() - (WIDTH_SHADOW - i),
                            RADIUS + i,
                            RADIUS + i);
        int alpha = 1 * (WIDTH_SHADOW - i + 1);
        color.setAlpha(alpha > 255 ? 255 : alpha);
        painter->setPen(color);
        painter->drawPath(path);
    }
    painter->restore();
}

void MenuStyleHelper::drawSeparator(const QStyleOptionMenuItem *option, QPainter *painter, const QWidget *widget) const
{
    Q_UNUSED(widget)
    if (option->menuItemType != QStyleOptionMenuItem::Separator)
        return;
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);
    painter->setBrush(ColorRepository::menuSeparatorColor());
    painter->drawRoundedRect(QRectF(option->rect.x() + option->rect.width() / 15,
                                    option->rect.center().y(),
                                    option->rect.width() - option->rect.width() / 15 * 2,
                                    HEIGHT_SEPARATOR),
                             HEIGHT_SEPARATOR / 2, HEIGHT_SEPARATOR / 2);
    painter->restore();
}

void MenuStyleHelper::drawBackgroundItem(const QStyleOptionMenuItem *option, QPainter *painter, const QWidget *widget) const
{
    Q_UNUSED(widget)
    if ((option->state & QStyle::State_Enabled) && (option->state & QStyle::State_MouseOver) || (option->state & QStyle::State_Selected))
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(Qt::NoPen);
        painter->setBrush(ColorRepository::menuHoveredBackgroundColor());
        painter->drawRoundedRect(option->rect.adjusted(WIDTH_BORDER,
                                                       WIDTH_BORDER,
                                                       -WIDTH_BORDER,
                                                       -WIDTH_BORDER),
                                 RADIUS, RADIUS);
        painter->restore();
    }
}

void MenuStyleHelper::drawCheckIndicator(const QStyleOptionMenuItem *option, QPainter *painter, const QWidget *widget) const
{
    if (!option->menuHasCheckableItems)
        return;
    painter->save();
    painter->setPen(ColorRepository::basicTextColor());
    QFont iconFont = QFont(ICONFONT_NAME);
    iconFont.setPixelSize(ICONFONT_SIZE);
    painter->setFont(iconFont);
    // TODO 计算精确位置
    painter->drawText(QRectF(option->rect.x() + option->rect.width() * 0.055,
                             option->rect.y(),
                             option->rect.width() * 0.055 * 0.7,
                             option->rect.height()),
                      Qt::AlignCenter, option->checked ? ICONFONT_Check : ICONFONT_None);
    painter->restore();
}

void MenuStyleHelper::drawText(const QStyleOptionMenuItem *option, QPainter *painter, const QWidget *widget) const
{
    Q_UNUSED(widget)
    if (option->text.isEmpty())
        return;
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QStringList listText = option->text.split("\t");
    if (!(option->state & QStyle::State_Enabled))
    {
        painter->setPen(ColorRepository::disabledTextColor());
    }
    else
    {
        painter->setPen(ColorRepository::basicTextColor());
    }
    painter->drawText(option->rect.adjusted(PADDING_ITEM,
                                            PADDING_ITEM,
                                            -PADDING_ITEM,
                                            -PADDING_ITEM),
                      Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine,
                      listText[0].replace("&", ""));
    if (listText.count() > 1)
    {
        painter->drawText(QRectF(option->rect.x() + option->rect.width() * 0.055 + 32 * 0.7 + 8,
                                 option->rect.y(),
                                 option->rect.width() - (option->rect.width() * 0.055 * 2 + 32 * 0.7 + 8),
                                 option->rect.height()),
                          Qt::AlignRight | Qt::AlignVCenter | Qt::TextSingleLine,
                          listText[1]);
    }
    painter->restore();
}

void MenuStyleHelper::drawIcon(int sizeIconOriginal, const QStyleOptionMenuItem *option, QPainter *painter, const QWidget *widget) const
{
    if (option->icon.isNull())
        return;
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->drawPixmap(QRect(option->rect.x() + PADDING_ITEM,
                              option->rect.y() + (option->rect.y() - sizeIconOriginal) / 2,
                              sizeIconOriginal,
                              sizeIconOriginal),
                        option->icon.pixmap(sizeIconOriginal, sizeIconOriginal));
    painter->restore();
}

void MenuStyleHelper::drawIconExpand(const QStyleOptionMenuItem *option, QPainter *painter, const QWidget *widget) const
{
    if (option->menuItemType != QStyleOptionMenuItem::SubMenu)
        return;
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    if (!(option->state & QStyle::State_Enabled))
    {
        painter->setPen(ColorRepository::disabledTextColor());
    }
    else
    {
        painter->setPen(ColorRepository::basicTextColor());
    }
    QFont iconFont = QFont(ICONFONT_NAME);
    iconFont.setPixelSize(ICONFONT_SIZE);
    painter->setFont(iconFont);
    painter->drawText(QRect(option->rect.right() - PADDING_ITEM,
                            option->rect.y(),
                            PADDING_ITEM,
                            option->rect.height()),
                      Qt::AlignVCenter,
                      ICONFONT_AngleRight);
    painter->restore();
}

QSize MenuStyleHelper::sizeMenuItem(const QStyleOptionMenuItem *option, QSize contentsSize, const QWidget *widget) const
{
    return QSize(qMax(contentsSize.width() + PADDING_ITEM * 2, WIDTH_MIN), contentsSize.height() + PADDING_ITEM * 2);
}