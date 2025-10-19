#include "PlainTextEditStyleHelper.h"
#include <QPlainTextEdit>
#include "ColorRepository.h"
#include <QPainterPath>

void PlainTextEditStyleHelper::drawBackgroundAndBorder(const QStyleOptionFrame *option, QPainter *painter, const QWidget *widget) const
{
    Q_UNUSED(widget)
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QPen borderPen;
    borderPen.setColor(ColorRepository::plainTextEditBorderColor());
    borderPen.setWidth(WIDTH_BORDER);
    painter->setPen(borderPen);
    painter->setBrush(ColorRepository::plainTextEditBackgroundColor());
    painter->drawRoundedRect(option->rect.adjusted(WIDTH_BORDER,
                                                   WIDTH_BORDER,
                                                   -WIDTH_BORDER,
                                                   -WIDTH_BORDER),
                             RADIUS, RADIUS);
    painter->restore();
}

void PlainTextEditStyleHelper::drawHemline(const QStyleOptionFrame *option, QPainter *painter, const QWidget *widget) const
{
    Q_UNUSED(widget)
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
    // 仅为QPlainTextEdit做偏移
    if (qobject_cast<const QPlainTextEdit *>(widget))
    {
        return option->rect.adjusted(WIDTH_BORDER + PADDING_HORIZONTAL,
                                     WIDTH_BORDER + PADDING_VERTICAL,
                                     -WIDTH_BORDER, // 右侧x坐标不偏移水平内边距，保证scrollbar紧贴右侧
                                     -WIDTH_BORDER - PADDING_VERTICAL);
    }
    return QRect();
}
