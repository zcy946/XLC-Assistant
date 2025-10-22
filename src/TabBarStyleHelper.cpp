#include "TabBarStyleHelper.h"
#include "XlcStyle.h"
#include "ColorRepository.h"
#include <QPainterPath>

void TabBarStyleHelper::drawBackground(const QStyleOptionTab *option, QPainter *painter, const QWidget *widget) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    painter->setPen(Qt::NoPen);
    QRect rectTab = option->rect;
    if (option->state.testFlag(QStyle::State_Selected))
    {
        // 选中背景绘制
        rectTab.setLeft(rectTab.left() - MARGIN_TAB);
        if (option->position != QStyleOptionTab::End)
        {
            rectTab.setRight(rectTab.right() + MARGIN_TAB + 1);
        }
        painter->setBrush(ColorRepository::basicSelectedAlphaColor());
        QPainterPath path;
        path.moveTo(rectTab.x(), rectTab.bottom() + 1);
        path.arcTo(QRectF(rectTab.x() - MARGIN_TAB,
                          rectTab.bottom() - MARGIN_TAB * 2 + 1,
                          MARGIN_TAB * 2,
                          MARGIN_TAB * 2),
                   -90, 90);
        path.lineTo(rectTab.x() + MARGIN_TAB, rectTab.y() + RADIUS_TAB);
        path.arcTo(QRectF(rectTab.x() + MARGIN_TAB,
                          rectTab.y(),
                          RADIUS_TAB * 2,
                          RADIUS_TAB * 2),
                   180, -90);
        path.lineTo(rectTab.right() - MARGIN_TAB - RADIUS_TAB, rectTab.y());
        path.arcTo(QRectF(rectTab.right() - MARGIN_TAB - 2 * RADIUS_TAB,
                          rectTab.y(),
                          RADIUS_TAB * 2,
                          RADIUS_TAB * 2),
                   90, -90);
        path.lineTo(rectTab.right() - MARGIN_TAB, rectTab.bottom() - MARGIN_TAB);
        path.arcTo(QRectF(rectTab.right() - MARGIN_TAB,
                          rectTab.bottom() - 2 * MARGIN_TAB + 1,
                          MARGIN_TAB * 2,
                          MARGIN_TAB * 2),
                   -180, 90);
        path.lineTo(rectTab.right(), rectTab.bottom() + (MARGIN_TAB + 1));
        path.lineTo(rectTab.x(), rectTab.bottom() + (MARGIN_TAB + 1));
        path.closeSubpath();
        painter->drawPath(path);
        // 绘制选中标记
        painter->setPen(Qt::NoPen);
        painter->setBrush(ColorRepository::primaryNormal());
        painter->drawRoundedRect(QRectF(rectTab.left() + MARGIN_TAB + MARGIN_MARK,
                                        rectTab.y() + MARGIN_MARK,
                                        WIDTH_MARK,
                                        rectTab.height() - MARGIN_MARK * 2),
                                 RADIUS_MARK, RADIUS_MARK);
    }
    else
    {
        if (option->state.testFlag(QStyle::State_MouseOver))
        {
            painter->setBrush(ColorRepository::basicHoveredAlphaColor());
        }
        else
        {
            painter->setBrush(Qt::transparent);
        }
        painter->drawRoundedRect(rectTab, RADIUS_TAB, RADIUS_TAB);
    }
    painter->restore();
}

void TabBarStyleHelper::drawLabel(const XlcStyle *style, const QStyleOptionTab *option, QPainter *painter, const QWidget *widget) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QRect rectTextOriginal = style->subElementRect(QStyle::SE_TabBarTabText, option, widget);
    rectTextOriginal.setLeft(rectTextOriginal.left() + (MARGIN_TAB + 1));
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    // 图标绘制
    QIcon icon = option->icon;
    if (!icon.isNull())
    {
        QRectF iconRect(option->rect.x() + 15, rectTextOriginal.center().y() - (qreal)option->iconSize.height() / 2 + 1, option->iconSize.width(), option->iconSize.height());
        QPixmap iconPix = icon.pixmap(option->iconSize,
                                      (option->state & QStyle::State_Enabled) ? QIcon::Normal
                                                                              : QIcon::Disabled,
                                      (option->state & QStyle::State_Selected) ? QIcon::On
                                                                               : QIcon::Off);
        painter->drawPixmap(iconRect.x(), iconRect.y(), iconPix);
    }
    // 文字绘制
    painter->setPen(ColorRepository::basicTextColor());
    QString text = painter->fontMetrics().elidedText(option->text, Qt::ElideRight, rectTextOriginal.width());
    painter->drawText(rectTextOriginal, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextDontClip, text);
    painter->restore();

    painter->restore();
}

QRect TabBarStyleHelper::rectText(const QStyleOptionTab *option, const QWidget *widget, QRect rectTextOriginal) const
{
    return rectTextOriginal;
}

QSize TabBarStyleHelper::sizeTab(const QStyleOptionTab *option, QSize sizeTabOriginal, const QWidget *widget) const
{
    return QSize(sizeTabOriginal.width() + PADDING_HORIZONTAL * 2, sizeTabOriginal.height() + PADDING_VERTICAL * 2);
}

void TabBarStyleHelper::drawTabWidgetBackground(const QStyleOptionTabWidgetFrame *option, QPainter *painter, const QWidget *widget) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(ColorRepository::basicBorderColor());
    painter->setBrush(ColorRepository::baseBackgroundColor());
    QRect rectBackground = option->rect.adjusted(WIDTH_BORDER,
                                                 WIDTH_BORDER,
                                                 -WIDTH_BORDER,
                                                 -WIDTH_BORDER);
    QPainterPath path;
    qreal radiusTL = 0.0;
    qreal radiusTR = 0.0;
    qreal radiusBL = RADIUS;
    qreal radiusBR = RADIUS;
    path.moveTo(rectBackground.topLeft());
    path.lineTo(rectBackground.topRight());
    path.lineTo(rectBackground.bottomRight().x(), rectBackground.bottomRight().y() - radiusBR);
    path.arcTo(QRectF(rectBackground.bottomRight().x() - 2 * radiusBR,
                      rectBackground.bottomRight().y() - 2 * radiusBR,
                      2 * radiusBR,
                      2 * radiusBR),
               0.0, -90.0);
    path.lineTo(rectBackground.bottomLeft().x() + radiusBL, rectBackground.bottomLeft().y());
    path.arcTo(QRectF(rectBackground.bottomLeft().x(),
                      rectBackground.bottomLeft().y() - 2 * radiusBL,
                      2 * radiusBL,
                      2 * radiusBL),
               270.0, -90.0);
    path.lineTo(rectBackground.topLeft());
    painter->drawPath(path);
    painter->restore();
}
