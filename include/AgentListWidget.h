#ifndef AGENTLISTWIDGET_H
#define AGENTLISTWIDGET_H

#include <QAbstractListModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QListView>
#include "global.h"

struct AgentItem
{
    QString uuid;          // 唯一标识符
    QString name;          // 名字
    int conversationCount; // 对话数量
};

class AgentListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit AgentListModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void addItem(const QString &uuid, const QString &name, int conversationCount);
    QModelIndex findIndex(const QString &uuid);
    void clearData();

public:
    enum Roles
    {
        Uuid = Qt::UserRole + 1,
        Name = Qt::UserRole + 2,
        ConversationCount = Qt::UserRole + 3
    };
    QVector<AgentItem> m_items;
};

// TODO 添加 `QSortFilterProxyModel` 实现通过最后更新时间排序

class AgentDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit AgentDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    const int MARGIN = 5;                              // item 外边距
    const int PADDING_HORIZONTAL = 10;                 // item 水平内边距
    const int PADDING_VERTICAL = 8;                    // item 垂直内边距
    const int OUTLINE_WIDTH = 2;                       // item 轮廓宽度
    const int SPACING_NAME_TO_CONVERSATION_COUNT = 5;  // name 到 对话计数背景之间的距离
    const int PADDING_CONVERSATION_COUNT = 2;          // 对话计数内边距
    const int SIZE_FONT = getGlobalFont().pointSize(); // 字体大小
    const int FONT_SIZE_CONVERSATION_COUNT = 8;        // 对话计数字体大小
};

class AgentListWidget : public QListView
{
    Q_OBJECT
Q_SIGNALS:
    void itemChanged(const QString &uuid);

public:
    explicit AgentListWidget(QWidget *parent = nullptr);
    void addItem(const QString &uuid, const QString &name, int conversationCount);
    void setCurrentAgent(const QString &uuid);
    void selectFirstAgent();
    QString currentAgentUuid();
    bool hasAgentSelected();
    // 清空列表
    void clear();

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    AgentListModel *m_model;
    AgentDelegate *m_delegate;
};

#endif // AGENTLISTWIDGET_H