#include "SpinBoxStyleHelper.h"
#include "ColorRepository.h"
#include <QPainterPath>
#include <QAbstractSpinBox>

void SpinBoxStyleHelper::drawBackground(const QStyleOptionSpinBox *option, QPainter *painter, const QWidget *widget)
{
    if (!qobject_cast<const QAbstractSpinBox *>(widget))
    {
        return;
    }
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QRect rectSpinBox = option->rect.adjusted(BORDER_WIDTH, BORDER_WIDTH, -BORDER_WIDTH, -BORDER_WIDTH);
    painter->setPen(ColorRepository::spinBoxBorderColor());
    painter->setBrush(ColorRepository::spinBoxBackgroundColor());
    painter->drawRoundedRect(rectSpinBox, RADIUS, RADIUS);
    painter->restore();
}

void SpinBoxStyleHelper::drawSubControls(const QStyleOptionSpinBox *option, QPainter *painter, const QWidget *widget, const QRect &rectSubLine, const QRect &rectAddLine)
{
    if (!qobject_cast<const QAbstractSpinBox *>(widget))
    {
        return;
    }
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);
    if (option->state & QStyle::State_Sunken && option->state & QStyle::State_MouseOver)
    {
        painter->setBrush(ColorRepository::spinBoxPressedArrowColor());
    }
    else if (option->state & QStyle::State_MouseOver)
    {
        painter->setBrush(ColorRepository::spinBoxHoveredArrowColor());
    }
    else
    {
        painter->setBrush(ColorRepository::spinBoxArrowColor());
    }
    QPainterPath path;
    // 增加按钮
    if (option->activeSubControls == QStyle::SC_ScrollBarAddLine)
    {
        QRectF rect(rectAddLine);
        path.moveTo(rect.topLeft());
        path.lineTo(rect.topRight() - QPointF(RADIUS, 0));
        path.arcTo(QRectF(rect.topRight() - QPointF(2 * RADIUS, 0), QSizeF(2 * RADIUS, 2 * RADIUS)), 90, -90);
        path.lineTo(rect.bottomRight());
        path.lineTo(rect.bottomLeft());
        path.closeSubpath();
        painter->drawPath(path);
        // painter->drawRoundedRect(rectAddLine, RADIUS, RADIUS);
    }
    // 减少按钮
    if (option->activeSubControls == QStyle::SC_ScrollBarSubLine)
    {
        QRectF rect(rectSubLine);
        path.moveTo(rect.topLeft());
        path.lineTo(rect.topRight());
        path.lineTo(rect.bottomRight() - QPointF(0, RADIUS));
        path.arcTo(QRectF(rect.bottomRight() - QPointF(2 * RADIUS, 2 * RADIUS), QSizeF(2 * RADIUS, 2 * RADIUS)), 0, -90);
        path.lineTo(rect.bottomLeft());
        path.closeSubpath();
        painter->drawPath(path);
        // painter->drawRoundedRect(rectSubLine, RADIUS, RADIUS);
    }
    // 绘制按钮上的图标
    // 增加图标
    QFont iconFont = QFont(ICONFONT_NAME);
    iconFont.setPixelSize(ICONFONT_SIZE);
    painter->setFont(iconFont);
    painter->setPen(ColorRepository::basicText());
    painter->drawText(rectAddLine, Qt::AlignCenter, ICONFONT_AngleUp);
    // 减小图标
    painter->drawText(rectSubLine, Qt::AlignCenter, ICONFONT_AngleDown);
    painter->restore();
}

void SpinBoxStyleHelper::drawHemline(const QStyleOptionSpinBox *option, QPainter *painter, const QWidget *widget)
{
    if (!qobject_cast<const QAbstractSpinBox *>(widget))
    {
        return;
    }
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);
    if (option->state & QStyle::State_HasFocus)
        painter->setBrush(ColorRepository::spinBoxFocusedHemlineColor());
    else
        painter->setBrush(ColorRepository::spinBoxHemlineColor());
    QPainterPath path;
    QRect rectSpinBox = option->rect;
    path.moveTo(MARGIN_HEMLINE, rectSpinBox.y() + rectSpinBox.height());
    path.lineTo(rectSpinBox.width() - MARGIN_HEMLINE, rectSpinBox.y() + rectSpinBox.height());
    path.arcTo(QRectF(rectSpinBox.width() - MARGIN_HEMLINE * 2,
                      rectSpinBox.y() + rectSpinBox.height() - MARGIN_HEMLINE * 2,
                      MARGIN_HEMLINE * 2,
                      MARGIN_HEMLINE * 2),
               -90, 45);
    path.lineTo(MARGIN_HEMLINE - 2 * std::sqrt(2), rectSpinBox.y() + rectSpinBox.height() - (MARGIN_HEMLINE - 2 * std::sqrt(2)));
    path.arcTo(QRectF(0, rectSpinBox.y() + rectSpinBox.height() - MARGIN_HEMLINE * 2, MARGIN_HEMLINE * 2, MARGIN_HEMLINE * 2), 225, 45);
    path.closeSubpath();
    painter->drawPath(path);
    painter->restore();
}
