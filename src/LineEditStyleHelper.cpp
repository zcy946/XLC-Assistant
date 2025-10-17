#include "LineEditStyleHelper.h"
#include "ColorRepository.h"
#include <QPainterPath>

void LineEditStyleHelper::drawLineEditShape(const QStyleOptionFrame *optionLineEdit, QPainter *painter, const QWidget *widget)
{
    Q_UNUSED(widget)
    // 背景
    drawBackground(optionLineEdit, painter);
    // 边框
    drawBorder(optionLineEdit, painter);
    // 底部边缘
    drawHemline(optionLineEdit, painter);
}

void LineEditStyleHelper::drawBackground(const QStyleOptionFrame *optionLineEdit, QPainter *painter)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QRect rectLineEdit = optionLineEdit->rect;
    if (optionLineEdit->state & QStyle::State_HasFocus)
    {
        painter->setBrush(ColorRepository::lineEditFocusedBackgroundColor());
    }
    else if (optionLineEdit->state & QStyle::State_MouseOver)
    {
        painter->setBrush(ColorRepository::lineEditHoveredBackgroundColor());
    }
    else
    {
        painter->setBrush(ColorRepository::lineEditBackgroundColor());
    }
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(rectLineEdit, RADIUS, RADIUS);
    painter->restore();
}

void LineEditStyleHelper::drawBorder(const QStyleOptionFrame *optionLineEdit, QPainter *painter)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QRect rectLineEdit = optionLineEdit->rect;
    painter->setPen(ColorRepository::lineEditBorderColor());
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(rectLineEdit.adjusted(BORDER_WIDTH, BORDER_WIDTH, -BORDER_WIDTH, -BORDER_WIDTH), RADIUS, RADIUS);
    painter->restore();
}

void LineEditStyleHelper::drawHemline(const QStyleOptionFrame *optionLineEdit, QPainter *painter)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QRect rectLineEdit = optionLineEdit->rect;
    if (optionLineEdit->state & QStyle::State_HasFocus)
    {
        painter->setBrush(ColorRepository::lineEditFocusedHemlineColor());
    }
    else
    {
        painter->setBrush(ColorRepository::lineEditHemlineColor());
    }
    QPainterPath path;
    path.moveTo(MARGIN_HEMLINE, rectLineEdit.height());
    path.lineTo(rectLineEdit.width() - MARGIN_HEMLINE, rectLineEdit.height());
    path.arcTo(QRectF(rectLineEdit.width() - MARGIN_HEMLINE * 2, rectLineEdit.height() - MARGIN_HEMLINE * 2, MARGIN_HEMLINE * 2, MARGIN_HEMLINE * 2), -90, 45);
    path.lineTo(MARGIN_HEMLINE - MARGIN_HEMLINE / 2 * std::sqrt(2), rectLineEdit.height() - (MARGIN_HEMLINE - MARGIN_HEMLINE / 2 * std::sqrt(2)));
    path.arcTo(QRectF(0, rectLineEdit.height() - MARGIN_HEMLINE * 2, MARGIN_HEMLINE * 2, MARGIN_HEMLINE * 2), 225, 45);
    path.closeSubpath();
    painter->setPen(Qt::NoPen);
    painter->drawPath(path);
    painter->restore();
}
