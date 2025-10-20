#ifndef XLCSTYLE_H
#define XLCSTYLE_H

#include <QProxyStyle>
#include <memory>
#include "PushButtonStyleHelper.h"
#include "ListViewStyleHelper.h"
#include "ItemViewItemStyleHelper.h"
#include "ScrollBarStyleHelper.h"
#include "LineEditStyleHelper.h"
#include "SpinBoxStyleHelper.h"
#include "PlainTextEditStyleHelper.h"
#include "CheckBoxStyleHelper.h"
#include "ComboBoxStyleHelper.h"
#include "GroupBoxStyleHelper.h"
#include <QStyledItemDelegate>

class ComboBoxDelegate;
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
    static ComboBoxDelegate *sharedComboBoxDelegate();

private:
    std::unique_ptr<PushButtonStyleHelper> m_pushButtonStyleHelper;
    std::unique_ptr<ListViewStyleHelper> m_listViewStyleHelper;
    std::unique_ptr<ItemViewItemStyleHelper> m_itemViewItemStyleHelper;
    std::unique_ptr<ScrollBarStyleHelper> m_scrollBarStyleHelper;
    std::unique_ptr<LineEditStyleHelper> m_lineEditStyleHelper;
    std::unique_ptr<SpinBoxStyleHelper> m_spinBoxStyleHelper;
    std::unique_ptr<PlainTextEditStyleHelper> m_plainTextEditStyleHelper;
    std::unique_ptr<CheckBoxStyleHelper> m_checkBoxStyleHelper;
    std::unique_ptr<ComboBoxStyleHelper> m_comboBoxStyleHelper;
    std::unique_ptr<GroupBoxStyleHelper> m_groupBoxStyleHelper;
};

class ComboBoxDelegate : public QStyledItemDelegate
{
public:
    explicit ComboBoxDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // XLCSTYLE_H