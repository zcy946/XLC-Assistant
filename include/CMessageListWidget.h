#ifndef CMESSAGELISTWIDGET_H
#define CMESSAGELISTWIDGET_H

#include <QString>
#include <QSize>
#include <QAbstractListModel>
#include <QVector>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QTextDocument>
#include <QListView>
#include "global.h"

constexpr char *DEFAULT_AVATAR = "://image/default_avatar.png";

struct CMessage
{
    QString id;
    QString text;
    enum Role
    {
        USER,
        ASSISTANT,
        SYSTEM
    };
    Role role;
    QString createdDateTime;
    QString avatarFilePath;

    // 缓存尺寸信息
    QSize cachedTextSize;
    QSize cachedItemSize;

    CMessage(const QString &id, const QString &text, Role role, const QString &createdDateTime, const QString &avatarFilePath)
        : id(id), text(text), role(role), createdDateTime(createdDateTime), avatarFilePath(avatarFilePath)
    {
    }

    CMessage(const QString &text, Role role = USER, const QString &createdDateTime = getCurrentDateTime(), const QString &avatarFilePath = DEFAULT_AVATAR)
        : id(generateUuid()), text(text), role(role), createdDateTime(createdDateTime)
    {
    }
};

class CMessageListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit CMessageListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public:
    enum MessageRoles
    {
        ID = Qt::UserRole + 1,
        Text = Qt::UserRole + 2,
        Role = Qt::UserRole + 3,
        CreatedDateTime = Qt::UserRole + 4,
        AvatarFilePath = Qt::UserRole + 5
    };
    void addMessage(const CMessage &message);

private:
    QVector<CMessage> m_messages;
};

class CMessageDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit CMessageDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    const char *COLOR_FONT_DATETIME = "#9E9E9E";
    const int AVATAR_SIZE = 40;             // 头像大小
    const int PADDING = 10;                 // 整体边距
    const int NICK_MARGIN = 5;              // 头像到昵称的距离
    const int TEXT_MARGIN = 15;             // 文本的上下外边距
};

class CMessageListWidget : public QListView
{
    Q_OBJECT
public:
    explicit CMessageListWidget(QWidget *parent = nullptr)
        : QListView(parent)
    {
    }

protected:
    void resizeEvent(QResizeEvent *event) override
    {
        QListView::resizeEvent(event);
        doItemsLayout(); // 强制让所有 item 重新调用 sizeHint()
    }
};

#endif // CMESSAGELISTWIDGET_H