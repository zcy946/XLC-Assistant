// --- START OF NEW CNavigationBar.h ---

#ifndef CNAVIGATIONBAR_H
#define CNAVIGATIONBAR_H

#include <QScrollArea>
#include <QPainter>
#include <QSvgRenderer>
#include <QMouseEvent>
#include <QResizeEvent>

class CNavigationContentWidget : public QWidget
{
    Q_OBJECT
Q_SIGNALS:
    void indexChanged(int index, const QString &text);

public:
    explicit CNavigationContentWidget(QWidget *parent = nullptr);
    void addItemSvg(const QString &text, const QString &filename);
    void addNonSelectableItemSvg(const QString &text, const QString &filename);

protected:
    QSize sizeHint() const override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override; // 添加 resizeEvent 处理

private:
    int itemAt(const QPoint &pos) const;
    void updateSize();

private:
    int m_marginLeft;                     // 边距 - 左
    int m_marginTop;                      // 边距 - 上
    int m_marginRight;                    // 边距 - 右
    int m_marginBottom;                   // 边距 - 下
    int m_spacingItem;                    // item间距离
    int m_borderRadiusBackground;         // icon背景圆角半径
    int m_spacingIcon2Background;         // icon与背景的间距
    int m_spacingBackground2Text;         // 背景与字体的间距
    int m_sizeFont;                       // 字体大小
    QColor m_colorFont;                   // 字体颜色
    QColor m_colorBackground;             // 背景颜色
    QColor m_colorIconBackgroundHovered;  // 悬停时icon背景颜色
    QColor m_colorIconBackgroundSelected; // 选中时icon背景颜色
    int m_heightText;                     // 当前字体下的字体高度
    int m_widthItem;                      // 每个item的宽度
    int m_heightItem;                     // 每个item的高度

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

class CNavigationBar : public QScrollArea
{
    Q_OBJECT
Q_SIGNALS:
    void indexChanged(int index, const QString &text);

public:
    explicit CNavigationBar(QWidget *parent = nullptr);
    ~CNavigationBar();
    void addItemSvg(const QString &text, const QString &filename);
    void addNonSelectableItemSvg(const QString &text, const QString &filename);

private:
    void initWidget();

private:
    CNavigationContentWidget *m_contentWidget;
};

#endif // CNAVIGATIONBAR_H