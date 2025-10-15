#ifndef XLCSTYLE_H
#define XLCSTYLE_H

#include <QProxyStyle>
#include "PushButtonStyleHelper.h"
#include <memory>

class XlcStyle : public QCommonStyle
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
    void polish(QWidget *w) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    std::unique_ptr<PushButtonStyleHelper> m_pushButtonStyleHelper;
};

#endif // XLCSTYLE_H