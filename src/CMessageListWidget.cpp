#include "CMessageListWidget.h"
#include <QPainterPath>
#include "Logger.hpp"

// CMessageListModel
CMessageListModel::CMessageListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int CMessageListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_messages.count();
}

QVariant CMessageListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_messages.size())
        return QVariant();

    const CMessage &message = m_messages.at(index.row());
    switch (role)
    {
    case MessageRoles::ID:
        return message.id;
    case MessageRoles::Text:
        return message.text;
    case MessageRoles::Role:
        return message.role;
    case MessageRoles::CreatedDateTime:
        return message.createdDateTime;
    case MessageRoles::AvatarFilePath:
        return message.avatarFilePath;
    case Qt::DecorationRole: // 用于直接加载潜在头像图片
        // 若图片较小，可在此处直接加载，
        // 或交由委托处理以获得更佳性能。
        return QVariant();
    default:
        return QVariant();
    }
}

void CMessageListModel::addMessage(const CMessage &message)
{
    beginInsertRows(QModelIndex(), m_messages.count(), m_messages.count());
    m_messages.append(message);
    endInsertRows();
}

// CMessageDelegate
CMessageDelegate::CMessageDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void CMessageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid())
        return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // 获取数据
    QString text = index.data(CMessageListModel::Text).toString();
    CMessage::Role role = static_cast<CMessage::Role>(index.data(CMessageListModel::Role).toInt());
    QString createDateTime = index.data(CMessageListModel::CreatedDateTime).toString();
    QString avatarFilePath = index.data(CMessageListModel::AvatarFilePath).toString();
    QString nick;
    if (role == CMessage::Role::USER)
        nick = "User";
    else if (role == CMessage::Role::ASSISTANT)
        nick = "Assistant";
    else if (role == CMessage::Role::SYSTEM)
        nick = "System";
    else
        nick = "Unknow";

    // 绘制头像
    QPixmap pixmapAvatar(avatarFilePath);
    if (pixmapAvatar.isNull())
    {
        pixmapAvatar.load(DEFAULT_AVATAR);
    }
    pixmapAvatar = pixmapAvatar.scaled(AVATAR_SIZE, AVATAR_SIZE, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPixmap pixmapRoundAvatar(AVATAR_SIZE, AVATAR_SIZE);
    pixmapRoundAvatar.fill(Qt::transparent);
    QPainterPath path;
    path.addEllipse(0, 0, AVATAR_SIZE, AVATAR_SIZE);
    QPainter roundPainter(&pixmapRoundAvatar);
    roundPainter.setRenderHint(QPainter::Antialiasing);
    roundPainter.setClipPath(path);
    roundPainter.drawPixmap(0, 0, pixmapAvatar);

    QRect rectAvatar(option.rect.topLeft() + QPoint(PADDING, PADDING), QSize(AVATAR_SIZE, AVATAR_SIZE));
    painter->drawPixmap(rectAvatar.topLeft(), pixmapRoundAvatar);

    QFontMetrics fontMetrics(option.font);

    // 绘制昵称
    QRect rectNick(rectAvatar.topRight() + QPoint(NICK_MARGIN, 0),
                   QPoint(option.rect.right() - PADDING, rectAvatar.top() + fontMetrics.height()));
    painter->setPen(Qt::black);
    painter->drawText(rectNick, nick);

    // 绘制datetime
    QFont fontDateTime(getGlobalFont());
    // fontDateTime.setPointSize(8);
    QRect rectDateTime(rectNick.left(),
                       rectAvatar.bottom() - QFontMetrics(fontDateTime).height(),
                       rectNick.width(),
                       QFontMetrics(fontDateTime).height());
    painter->setPen(QColor(COLOR_FONT_DATETIME));
    painter->setFont(fontDateTime);
    painter->drawText(rectDateTime, createDateTime);

    // 绘制文本
    painter->setPen(Qt::black);
    painter->setFont(option.font);
    int textWidth = option.rect.width() - rectAvatar.width() - PADDING * 3;
    QRect rectText = fontMetrics.boundingRect(0, 0, textWidth, 0, Qt::TextWordWrap, text);
    QRect rectDrawText(rectAvatar.bottomRight() + QPoint(NICK_MARGIN, TEXT_MARGIN), rectText.size());
    // 使用 QTextLayout 可实现更优的文本处理，尤其适用于富文本或复杂对齐场景
    // 对于纯文本，drawText 即可满足需求。
    painter->drawText(rectDrawText, Qt::TextWordWrap, text);

    painter->restore();
}

QSize CMessageDelegate::sizeHint(const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    if (!index.isValid())
        return QSize();

    // 获取用于计算的数据
    QString text = index.data(CMessageListModel::Text).toString();

    // 拿到 listView 的宽度（比 option.rect 更准确）
    int viewWidth = option.widget ? option.widget->width() : option.rect.width();

    // 头像区域高度
    int avatarHeight = AVATAR_SIZE;

    // 文本区域宽度
    QFontMetrics fontMetrics(option.font);
    int textWidth = viewWidth - PADDING - AVATAR_SIZE - NICK_MARGIN - PADDING; // 左 padding + 头像 + 头像到昵称的距离 + 右 padding

    if (textWidth < 50) // 宽度太小保护一下
        textWidth = 50;

    // 计算文本高度
    QRect rectText = fontMetrics.boundingRect(0, 0, textWidth, 0,
                                              Qt::TextWordWrap, text);
    int textHeight = rectText.height();
    int totalHeight = PADDING + avatarHeight + TEXT_MARGIN + textHeight + TEXT_MARGIN; // 上 padding + 头像 + 文本外边距 + 文本 + 文本外边距
    return QSize(viewWidth, totalHeight);
}
