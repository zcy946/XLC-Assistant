#include "ComboBoxStyleHelper.h"
#include "XlcStyle.h"
#include "ColorRepository.h"
#include <QPainterPath>

void ComboBoxStyleHelper::drawBackground(const XlcStyle *style, const QStyleOptionComboBox *option, QPainter *painter, const QWidget *widget) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    // 背景绘制
    painter->setPen(ColorRepository::comboBoxBorderColor());
    if (option->state & QStyle::State_Enabled)
    {
        if ((option->state & QStyle::State_HasFocus) && option->editable)
            painter->setBrush(ColorRepository::comboBoxEditedBackgroundColor());
        else if (option->state & QStyle::State_MouseOver)
            painter->setBrush(ColorRepository::comboBoxHoveredBackgroundColor());
        else
            painter->setBrush(ColorRepository::comboBoxBackgroundColor());
    }
    else
    {
        painter->setBrush(ColorRepository::comboBoxDisabledBackgroundColor());
    }
    painter->drawRoundedRect(option->rect.adjusted(WIDTH_BORDER,
                                                   WIDTH_BORDER,
                                                   -WIDTH_BORDER,
                                                   -WIDTH_BORDER),
                             RADIUS, RADIUS);
    painter->restore();
}

void ComboBoxStyleHelper::drawText(const QStyleOptionComboBox *option, QPainter *painter, const QWidget *widget) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    // 文字绘制
    if (!option->editable)
    {
        QRect textRect = option->rect.adjusted(PADDING_HORIZONTAL, PADDING_VERTICAL, -PADDING_HORIZONTAL, -PADDING_VERTICAL);
        if (option->state & QStyle::State_Enabled)
            painter->setPen(ColorRepository::basicText());
        else
            painter->setPen(ColorRepository::basicDisableText());
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignBottom, option->currentText);
    }
    painter->restore();
}

void ComboBoxStyleHelper::drawHemline(const QStyleOptionComboBox *option, QPainter *painter, const QWidget *widget) const
{
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

void ComboBoxStyleHelper::drawArrow(const XlcStyle *style, const QStyleOptionComboBox *option, QPainter *painter, const QWidget *widget) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QRect rectExpandIcon = style->subControlRect(QStyle::CC_ComboBox, option, QStyle::SC_ComboBoxArrow, widget);
    QFont iconFont = QFont(ICONFONT_NAME);
    iconFont.setPixelSize(ICONFONG_SIZE);
    painter->setFont(iconFont);
    if (option->state.testFlag(QStyle::State_Enabled))
        painter->setPen(ColorRepository::basicText());
    else
        painter->setPen(ColorRepository::basicDisableText());
    painter->drawText(rectExpandIcon, Qt::AlignCenter, ICONFONT_AngleDown);
    painter->restore();
}

void ComboBoxStyleHelper::drawContainerBackground(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    Q_UNUSED(widget)
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QStyleOptionComboBox *optionComBoBox = new QStyleOptionComboBox();
    optionComBoBox->rect = option->rect;
    QRect rect = option->rect;
    /**
     * BUG 绘制透明色会导致动画期间黑色
        drawShadow(painter, rect);
        QRect rectForeground(rect.x() + WIDTH_BORDER + WIDTH_SHADOW_BORDER,
                            rect.y() + WIDTH_BORDER,
                            rect.width() - WIDTH_SHADOW_BORDER * 2 - WIDTH_BORDER * 2,
                            rect.height() - WIDTH_SHADOW_BORDER - WIDTH_BORDER * 2);
        painter->setPen(ColorRepository::popupBorderColor());
        painter->setBrush(ColorRepository::popupBackgroundColor());
        painter->drawRoundedRect(rectForeground, RADIUS, RADIUS);
     */
    painter->setPen(ColorRepository::popupBorderColor());
    painter->setBrush(ColorRepository::popupBackgroundColor());
    painter->drawRect(rect);
    painter->restore();
    return;
}

QSize ComboBoxStyleHelper::sizeFromContents(const QStyleOptionComboBox *option, QSize contentsSize, const QWidget *widget) const
{
    return QSize(contentsSize.width() + PADDING_HORIZONTAL * 2, contentsSize.height() + PADDING_VERTICAL * 2);
}

QRect ComboBoxStyleHelper::rectFrame(const QStyleOptionComboBox *option, const QWidget *widget, QRect rectFrameOriginal) const
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    return rectFrameOriginal;
}

QRect ComboBoxStyleHelper::rectEditField(const QStyleOptionComboBox *option, const QWidget *widget) const
{
    Q_UNUSED(widget)
    return option->rect;
}

QRect ComboBoxStyleHelper::rectArrow(const QStyleOptionComboBox *option, const QWidget *widget) const
{
    Q_UNUSED(widget)
    QFont iconfont(ICONFONT_NAME);
    iconfont.setPixelSize(ICONFONG_SIZE);
    QFontMetrics fm(iconfont);
    int heightIconFont = fm.height();
    int widthIconFont = fm.horizontalAdvance(ICONFONT_AngleDown);
    int x = option->rect.width() - PADDING_ARROW - widthIconFont;
    int y = (option->rect.height() - heightIconFont) / 2;
    QRect rectArrow = QRect(x, y, widthIconFont, heightIconFont);
    return rectArrow;
}

QRect ComboBoxStyleHelper::rectPopup(const QStyleOptionComboBox *option, const QWidget *widget, QRect rectPopupOriginal) const
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    // BUG 绘制透明色会导致动画期间黑色
    // return rectPopupOriginal.adjusted(-WIDTH_SHADOW_BORDER - WIDTH_BORDER,
    //                                   SPACING_CONTENT_TO_POPUP,
    //                                   WIDTH_SHADOW_BORDER + WIDTH_BORDER,
    //                                   SPACING_CONTENT_TO_POPUP);
    return rectPopupOriginal.adjusted(0, SPACING_CONTENT_TO_POPUP, 0, SPACING_CONTENT_TO_POPUP);
}

int ComboBoxStyleHelper::margin() const
{
    return WIDTH_SHADOW_BORDER + WIDTH_BORDER;
}

void ComboBoxStyleHelper::drawShadow(QPainter *painter, QRect rect) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    QColor color = ColorRepository::shadowColor();
    for (int i = 0; i < WIDTH_SHADOW_BORDER; i++)
    {
        path.addRoundedRect(rect.x() + WIDTH_SHADOW_BORDER - i,
                            rect.y(),
                            rect.width() - (WIDTH_SHADOW_BORDER - i) * 2,
                            rect.height() - (WIDTH_SHADOW_BORDER - i),
                            RADIUS + i,
                            RADIUS + i);
        int alpha = 1 * (WIDTH_SHADOW_BORDER - i + 1);
        color.setAlpha(alpha > 255 ? 255 : alpha);
        painter->setPen(color);
        painter->drawPath(path);
    }
    painter->restore();
}