#include "PlainTextEditStyleHelper.h"
#include <QPlainTextEdit>
#include "ColorRepository.h"
#include <QPainterPath>

void PlainTextEditStyleHelper::drawBackgroundAndBorder(const QStyleOptionFrame *option, QPainter *painter, const QWidget *widget) const
{
    if (!qobject_cast<const QPlainTextEdit *>(widget))
    {
        return;
    }
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QPen borderPen;
    borderPen.setColor(ColorRepository::plainTextEditBorderColor());
    borderPen.setWidth(BORDER_WIDTH);
    painter->setPen(borderPen);
    painter->setBrush(ColorRepository::plainTextEditBackgroundColor());
    painter->drawRoundedRect(option->rect.adjusted(BORDER_WIDTH,
                                                   BORDER_WIDTH,
                                                   -BORDER_WIDTH,
                                                   -BORDER_WIDTH),
                             RADIUS, RADIUS);
    painter->restore();
}

void PlainTextEditStyleHelper::drawHemline(const QStyleOptionFrame *option, QPainter *painter, const QWidget *widget) const
{
    if (!qobject_cast<const QPlainTextEdit *>(widget))
    {
        return;
    }
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);
    int marginLB = MARGIN_HEMLINE;
    if (option->state & QStyle::State_HasFocus)
    {
        painter->setBrush(ColorRepository::spinBoxFocusedHemlineColor());
        marginLB = MARGIN_HEMLINE_FOCUSED;
    }
    else
    {
        painter->setBrush(ColorRepository::spinBoxHemlineColor());
    }
    QPainterPath path;
    QRect rectSpinBox = option->rect;
    path.moveTo(marginLB, rectSpinBox.y() + rectSpinBox.height());
    path.lineTo(rectSpinBox.width() - marginLB, rectSpinBox.y() + rectSpinBox.height());
    path.arcTo(QRectF(rectSpinBox.width() - marginLB * 2,
                      rectSpinBox.y() + rectSpinBox.height() - marginLB * 2,
                      marginLB * 2,
                      marginLB * 2),
               -90, 45);
    path.lineTo(marginLB - marginLB / 2 * std::sqrt(2), rectSpinBox.y() + rectSpinBox.height() - (marginLB - marginLB / 2 * std::sqrt(2)));
    path.arcTo(QRectF(0, rectSpinBox.y() + rectSpinBox.height() - marginLB * 2, marginLB * 2, marginLB * 2), 225, 45);
    path.closeSubpath();
    painter->drawPath(path);
    painter->restore();
}

QRect PlainTextEditStyleHelper::contentArea(const QStyleOptionFrame *option, const QWidget *widget) const
{
    if (qobject_cast<const QPlainTextEdit *>(widget))
    {
        return option->rect.adjusted(BORDER_WIDTH + PADDING_HORIZONTAL,
                                     BORDER_WIDTH + PADDING_VERTICAL,
                                     -BORDER_WIDTH, // 右侧x坐标不偏移水平内边距，保证scrollbar紧贴右侧
                                     -BORDER_WIDTH - PADDING_VERTICAL);
    }
    return QRect();
}
