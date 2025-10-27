#ifndef XLCNAVIGATIONBAR_H
#define XLCNAVIGATIONBAR_H

#include "BaseWidget.hpp"
#include <QAbstractItemModel>
#include <QList>
#include <QTreeView>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QStyledItemDelegate>

class XlcNavigationTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit XlcNavigationTreeView(QWidget *parent = nullptr);
    // ancestor 是否是 child 的祖先
    bool isAncestorOf(const QModelIndex &ancestor, const QModelIndex &child) const;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    // 被选中的叶子节点
    QModelIndex m_modelIndexSelectedLeaf;
};

class XlcNavigationNode
{
public:
    enum Role
    {
        Title = Qt::UserRole + 1,
        TargetId = Qt::UserRole + 2,
        Iconfont = Qt::UserRole + 3
    };
    explicit XlcNavigationNode(const QString &title, const QChar &iconfont, const QString &targetId, XlcNavigationNode *parent);
    ~XlcNavigationNode();
    void appendChild(XlcNavigationNode *child);
    XlcNavigationNode *child(int row);
    int childCount() const;
    QString title() const;
    QString targetId() const;
    QChar iconfont() const;
    int row() const;
    XlcNavigationNode *parentNode();

private:
    QString m_title;    // 节点标题(用于展示)
    QString m_targetId; // 节点ID
    QChar m_iconfont;   // 字体图标
    QList<XlcNavigationNode *> m_childNodes;
    XlcNavigationNode *m_parentNode;
};

class XlcNavigationModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit XlcNavigationModel(QObject *parent = nullptr);
    ~XlcNavigationModel() override;
    void addNavigationNode(const QString &title, const QChar &iconfont, const QString &targetId, const QModelIndex &parent = QModelIndex());
    QModelIndex findIndexByTitle(const QString &title, const QModelIndex &startParent = QModelIndex()) const;

    // 核心的纯虚函数实现
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    // 根据 QModelIndex 找到对应的 XlcNavigationNode
    XlcNavigationNode *getNode(const QModelIndex &index) const;
    // 根据 title 找到对应的 QModelIndex
    QModelIndex recursiveFind(const QString &title, const QModelIndex &parentIndex) const;
    XlcNavigationNode *m_rootNode;
};

class XlcNavigationDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit XlcNavigationDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    const QString ICONFONT_NAME = "ElaAwesome";     // 字体图标名称
    const QChar ICONFONT_AngleDown = QChar(0xe832); // 字体图标 - 下箭头
    const QChar ICONFONT_AngleUp = QChar(0xe839);   // 字体图标 - 上箭头
    const int ICONFONT_SIZE = 17;                   // 字体图标大小
    const int SPACING_ICON_TO_TEXT = 10;            // 字体图标到文字之间的距离
    const int MARGIN_INDICATOR_HORIZONTIAL = 12;    // 图标&展开/折叠指示器水平外边距
    const int OFFSET_MARK_X = 3;                    // 选中标记水平偏移(左侧蓝色圆角矩形)
    const int WIDTH_MARK = 3;                       // 选中标记宽度
    const int RADIUS_MARK = 3;                      // 选中标记圆角半径
    const qreal PADDING_MARK_TOP = 7;               // 选中标记上侧内边距
    const qreal PADDING_MARK_BOTTOM = 7;            // 选中标记下侧内边距
    const int SPACING_MARK_TO_TEXT = 5;             // 选中标记到文本之间的距离
    const int SPACING_TOP = 2;                      // 不用于计算绘制的到上一个item的距离(不绘制 SPACING_TOP 这段距离，视觉效果就像setSpacing(SPACING_TOP))
    const int RADIUS = 4;                           // 圆角半径
    const int PADDING_VERTICAL = 4;                 // XlcNavigation中的item相较于普通item额外增加的内边距
};

class XlcNavigationBar : public BaseWidget
{
    Q_OBJECT
Q_SIGNALS:
    void sig_currentItemChanged(const QString &targetId);

private Q_SLOTS:
    void slot_handleCurrentItemChanged(const QModelIndex &current, const QModelIndex &previous);

public:
    explicit XlcNavigationBar(BaseWidget *parent = nullptr);
    void addNavigationNode(const QString &title, const QChar &iconfont, const QString &targetId = QString(), const QString &parentTitle = QString());
    // 设置选中项
    void setSelectedItem(const QString &title);

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;

private:
    XlcNavigationModel *m_navigationModel;
    XlcNavigationTreeView *m_treeView;
};

#endif // XLCNAVIGATIONBAR_H