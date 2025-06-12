#ifndef CNAVIGATIONBAR_H
#define CNAVIGATIONBAR_H

#include "BaseWidget.hpp"
#include <QVector>

class CNavigationBar : public BaseWidget
{
    Q_OBJECT
Q_SIGNALS:
    void indexChanged(int index, const QString &name);

public:
    explicit CNavigationBar(QWidget *parent = nullptr);
    ~CNavigationBar();
    void addItemSvg(const QString &text, const QString &filename);
    void addNonSelectableItemSvg(const QString &text, const QString &filename);

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_marginLeft = 10;         // 外边距 - 左
    int m_marginTop = 10;          // 外边距 - 上
    int m_marginRight = 10;        // 外边距 - 右
    int m_marginBottom = 0;        // 外边距 - 下
    int m_spacing = 10;             // item 间距
    int m_borderRadius;            // icon 背景圆角半径大小
    int m_sizeFont;                // 文字大小
    int m_colorFont;               // 文字颜色
    int m_colorBackground;         // icon 背景颜色
    int m_colorBackgroundHovered;  // icon 背景hover时的颜色
    int m_colorBackgroundSelected; // icon 背景selected时的颜色

private:
    struct item
    {
        const QString text;
        const QString filename;
        const bool selectable;
    };
    QVector<item> m_items;
    int m_indexHovered;
    int m_indexSelected;
};

#endif // CNAVIGATIONBAR_H