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
    mutable QSize cachedTextSize;
    mutable QSize cachedItemSize;

    CMessage(const QString &id, const QString &text, Role role, const QString &avatarFilePath, const QString &createdDateTime)
        : id(id), text(text), role(role), avatarFilePath(avatarFilePath), createdDateTime(createdDateTime)
    {
    }

    CMessage(const QString &text, Role role = USER, const QString &avatarFilePath = DEFAULT_AVATAR, const QString &createdDateTime = getCurrentDateTime())
        : id(generateUuid()), text(text), role(role), avatarFilePath(avatarFilePath), createdDateTime(createdDateTime)
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
    enum MessageRoles
    {
        ID = Qt::UserRole + 1,
        Text = Qt::UserRole + 2,
        Role = Qt::UserRole + 3,
        CreatedDateTime = Qt::UserRole + 4,
        AvatarFilePath = Qt::UserRole + 5
    };
    void addMessage(const CMessage &message);
    const CMessage *messageAt(int row) const;
    void clearCachedSizes();

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
    QPixmap getRoundedAvatar(const QString &avatarFilePath, int size) const;

private:
    const char *COLOR_FONT_DATETIME = "#9E9E9E"; // 时间戳的字体颜色
    const int AVATAR_SIZE = 40;                  // 头像大小
    const int PADDING = 10;                      // 整体边距
    const int NICK_MARGIN = 10;                  // 头像到昵称的距离
    const int TEXT_MARGIN = 15;                  // 文本的上下外边距
};

class CMessageListWidget : public QListView
{
    Q_OBJECT
public:
    explicit CMessageListWidget(QWidget *parent = nullptr);
    void addMessage(const CMessage &message);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    CMessageListModel *m_model;
    CMessageDelegate *m_delegate;
};

#endif // CMESSAGELISTWIDGET_H