#include "LineEditStyleHelper.h"
#include "ColorRepository.h"
#include <QPainterPath>
#include <QLineEdit>
#include <QAbstractSpinBox>

void LineEditStyleHelper::drawLineEditShape(const QStyleOptionFrame *optionLineEdit, QPainter *painter, const QWidget *widget)
{
    // 检查请求是否由 QLineEdit 发起
    const QLineEdit *lineEdit = qobject_cast<const QLineEdit *>(widget);
    if (!lineEdit)
        return;
    /**
     * QAbstractSpinBox(QSpinBox和QDoubleSpinbox)内嵌了一个QLineEdit，在绘制背景这块排除它们
     *  */
    if (qobject_cast<const QAbstractSpinBox *>(lineEdit->parentWidget()))
        return;
    // 背景
    drawBackground(optionLineEdit, painter);
    // 边框
    drawBorder(optionLineEdit, painter);
    // 底部边缘
    drawHemline(optionLineEdit, painter);
}

QRect LineEditStyleHelper::subElementRect(QStyle::SubElement subElement, const QStyleOptionFrame *option, const QWidget *widget, const QRect &rectBasic)
{
    if (qobject_cast<const QLineEdit *>(widget))
    {
        return rectBasic.adjusted(PADDING_HORIZONTAL, PADDING_VERTICAL, -PADDING_HORIZONTAL, -PADDING_VERTICAL);
    }
    return rectBasic;
}

QSize LineEditStyleHelper::sizeFromContents(const QStyleOptionFrame *option, QSize sizeBasic, const QWidget *widget)
{
    if (qobject_cast<const QLineEdit *>(widget))
    {
        sizeBasic.rwidth() += PADDING_HORIZONTAL * 2;
        sizeBasic.rheight() += PADDING_VERTICAL * 2;
    }
    return sizeBasic;
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
    painter->drawRoundedRect(rectLineEdit.adjusted(WIDTH_BORDER, WIDTH_BORDER, -WIDTH_BORDER, -WIDTH_BORDER), RADIUS, RADIUS);
    painter->restore();
}

void LineEditStyleHelper::drawHemline(const QStyleOptionFrame *optionLineEdit, QPainter *painter)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);
    QRect rectLineEdit = optionLineEdit->rect;
    int marginLB = MARGIN_HEMLINE;
    if (optionLineEdit->state & QStyle::State_HasFocus)
    {
        painter->setBrush(ColorRepository::lineEditFocusedHemlineColor());
        marginLB = MARGIN_HEMLINE_FOCUSED;
    }
    else
    {
        painter->setBrush(ColorRepository::lineEditHemlineColor());
    }
    QPainterPath path;
    path.moveTo(marginLB, rectLineEdit.height());
    path.lineTo(rectLineEdit.width() - marginLB, rectLineEdit.height());
    path.arcTo(QRectF(rectLineEdit.width() - marginLB * 2,
                      rectLineEdit.height() - marginLB * 2,
                      marginLB * 2,
                      marginLB * 2),
               -90, 45);
    path.lineTo(marginLB - marginLB / 2 * std::sqrt(2), rectLineEdit.height() - (marginLB - marginLB / 2 * std::sqrt(2)));
    path.arcTo(QRectF(0, rectLineEdit.height() - marginLB * 2, marginLB * 2, marginLB * 2), 225, 45);
    path.closeSubpath();
    painter->drawPath(path);
    painter->restore();
}
