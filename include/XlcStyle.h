#ifndef XLCSTYLE_H
#define XLCSTYLE_H

#include <QProxyStyle>
#include <memory>
#include "PushButtonStyleHelper.h"
#include "ItemViewItemStyleHelper.h"
#include "ScrollBarStyleHelper.h"
#include "LineEditStyleHelper.h"

class XlcStyle : public QProxyStyle
{
public:
    XlcStyle();
    void drawPrimitive(PrimitiveElement pe, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override;
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override;
    void drawComplexControl(ComplexControl complexControl, const QStyleOptionComplex *opt, QPainter *painter, const QWidget *widget = nullptr) const override;
    int pixelMetric(PixelMetric metric, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override;
    int styleHint(StyleHint stylehint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &contentsSize, const QWidget *widget) const override;
    QRect subElementRect(SubElement subElement, const QStyleOption *option, const QWidget *widget) const override;
    QRect subControlRect(ComplexControl complexControl, const QStyleOptionComplex *option, SubControl subControl, const QWidget *widget) const override;
    void polish(QWidget *w) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    std::unique_ptr<PushButtonStyleHelper> m_pushButtonStyleHelper;
    std::unique_ptr<ItemViewItemStyleHelper> m_itemViewItemStyleHelper;
    std::unique_ptr<ScrollBarStyleHelper> m_scrollBarStyleHelper;
    std::unique_ptr<LineEditStyleHelper> m_lineEditStyleHelper;
};

#endif // XLCSTYLE_H