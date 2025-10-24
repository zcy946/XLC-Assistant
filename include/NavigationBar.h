#ifndef NAVIGATIONBAR_H
#define NAVIGATIONBAR_H

#include <QAbstractListModel>
#include <QStyledItemDelegate>
#include <QListView>
#include "global.h"

struct NavigationItem
{
    QString text;
    QString iconFilePath;
};

class NavigationItemListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit NavigationItemListModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    // Qt::ItemFlags flags(const QModelIndex &index) const override;
    // void addItem(const QString &text, const QString &iconFilePath, bool selectable);
    void addItem(const QString &text, const QString &iconFilePath);
    QModelIndex findIndexByText(const QString &text) const;

public:
    enum Roles
    {
        Text = Qt::UserRole + 1,
        IconFilePath = Qt::UserRole + 2,
        // Selectable = Qt::UserRole + 3
    };
    QVector<NavigationItem> m_items;
};

class NavigationItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit NavigationItemDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    const int RADIUS = 5;                                  // item 圆角边框半径
    const int MARGIN = 5;                                  // item 外边距
    const int PADDING = 5;                                 // item 内边距
    const int SPACING_ICON_TO_TEXT = 2;                    // item 图标(包含背景)到文本之间的距离
    const QString DEFAULT_ICON = "://image/fire-line.svg"; // 图标加载失败时的默认图标
    const int SIZE_FONT = getGlobalFont().pointSize();     // 字体大小
    const int OUTLINE_WIDTH = 2;                           // item 外边框宽度
    const int MARGIN_MARK = 2;                             // 选中标记外边距
    const int WIDTH_MARK = 3;                              // 选中标记宽度
    const int RADIUS_MARK = 2;                             // 选中标记圆角半径
};

class NavigationBar : public QListView
{
    Q_OBJECT
Q_SIGNALS:
    void indexChanged(int index);

public:
    explicit NavigationBar(QWidget *parent = nullptr);
    // void addItem(const QString &text, const QString &iconFilePath, bool selectable = true);
    void addItem(const QString &text, const QString &iconFilePath);
    void setCurrentText(const QString &text);

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    NavigationItemListModel *m_model;
    NavigationItemDelegate *m_delegate;
};

#endif // NAVIGATIONBAR_H