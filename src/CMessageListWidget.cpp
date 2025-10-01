#include "CMessageListWidget.h"
#include <QPainterPath>
#include <QPixmapCache>
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
        return message.content;
    case MessageRoles::Role:
        return message.role;
    case MessageRoles::CreatedTime:
        return message.createdTime;
    case MessageRoles::AvatarFilePath:
        return message.avatarFilePath;
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

const CMessage *CMessageListModel::messageAt(int row) const
{
    if (row >= m_messages.size())
        return nullptr;
    return &m_messages.at(row);
}

void CMessageListModel::clearCachedSizes()
{
    for (CMessage &message : m_messages)
        message.cachedItemSize = QSize();
}

void CMessageListModel::clearAllMessage()
{
    if (m_messages.isEmpty())
    {
        return;
    }
    beginInsertRows(QModelIndex(), 0, m_messages.count() - 1);
    m_messages.clear();
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
    Message::Role role = static_cast<Message::Role>(index.data(CMessageListModel::Role).toInt());

    // 绘制清除上下文分割线
    if (role == Message::SYSTEM && text == DEFAULT_CONTENT_CLEAR_CONTEXT)
    {
        QString text = "清除上下文";
        QFontMetrics fontMetrics(option.font);
        int textWidth = fontMetrics.horizontalAdvance(text);
        int lineLength = (option.rect.width() - textWidth) / 2;
        QRect rectText = fontMetrics.boundingRect(0, 0, textWidth, 0, Qt::TextWordWrap, text);
        QRect rectDrawText = QRect(option.rect.topLeft() + QPoint(lineLength, PADDING), rectText.size());
        painter->setPen(QColor("#D0D0D0"));
        painter->drawLine(QPoint(option.rect.left() + PADDING, option.rect.top() + PADDING + fontMetrics.height() / 2), QPoint(rectDrawText.left() - PADDING, option.rect.top() + PADDING + fontMetrics.height() / 2));
        painter->setPen(QColor("#000000"));
        painter->drawText(rectDrawText, text);
        painter->setPen(QColor("#D0D0D0"));
        painter->drawLine(QPoint(rectDrawText.right() + PADDING, option.rect.top() + PADDING + fontMetrics.height() / 2), QPoint(option.rect.right() - PADDING, option.rect.top() + PADDING + fontMetrics.height() / 2));
        painter->restore();
        return;
    }

    QString createDateTime = index.data(CMessageListModel::CreatedTime).toString();
    QString avatarFilePath = index.data(CMessageListModel::AvatarFilePath).toString();
    QString nick;
    if (role == Message::Role::USER)
        nick = "User";
    else if (role == Message::Role::ASSISTANT)
        nick = "Assistant";
    else if (role == Message::Role::TOOL)
        nick = "Tool";
    else if (role == Message::Role::SYSTEM)
        nick = "System";
    else
        nick = "Unknown";

    // 绘制头像
    QRect rectAvatar(option.rect.topLeft() + QPoint(PADDING, PADDING), QSize(AVATAR_SIZE, AVATAR_SIZE));
    QPixmap avatar = getRoundedAvatar(avatarFilePath, AVATAR_SIZE);
    painter->drawPixmap(rectAvatar, avatar);

    QFontMetrics fontMetrics(option.font);

    // 绘制昵称
    QRect rectNick(rectAvatar.topRight() + QPoint(NICK_MARGIN, 0),
                   QPoint(option.rect.right() - PADDING, rectAvatar.top() + fontMetrics.height()));
    painter->setPen(Qt::black);
    painter->drawText(rectNick, nick);

    // 绘制时间戳
    QFont fontDateTime(getGlobalFont());
    fontDateTime.setPointSize(8);
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

    // 获取message
    CMessage *message = const_cast<CMessage *>(
        static_cast<const CMessageListModel *>(index.model())->messageAt(index.row()));

    if (!message->cachedItemSize.isEmpty())
    {
        return message->cachedItemSize;
    }

    // 拿到 listView 的宽度
    int viewWidth = option.rect.width();
    QFontMetrics fontMetrics(option.font);

    // 清除上下文分割线
    if (message->role == Message::SYSTEM && message->content == DEFAULT_CONTENT_CLEAR_CONTEXT)
    {
        return QSize(viewWidth, fontMetrics.height() + PADDING * 2);
    }

    // 正常消息
    int textWidth = viewWidth - PADDING - AVATAR_SIZE - NICK_MARGIN - PADDING; // 左 padding + 头像 + 头像到昵称的距离 + 右 padding
    if (textWidth < 50)                                                        // 宽度太小保护一下
        textWidth = 50;
    QRect rectText = fontMetrics.boundingRect(0, 0, textWidth, 0, Qt::TextWordWrap, message->content);

    int avatarHeight = AVATAR_SIZE;
    int textHeight = rectText.height();
    int totalHeight = PADDING + avatarHeight + TEXT_MARGIN + textHeight + TEXT_MARGIN; // 上 padding + 头像 + 文本外边距 + 文本 + 文本外边距

    message->cachedItemSize = QSize(viewWidth, totalHeight);
    return message->cachedItemSize;
}

QPixmap CMessageDelegate::getRoundedAvatar(const QString &avatarFilePath, int size) const
{
    // 生成缓存 key（路径 + 尺寸）
    QString key = QString("%1_%2").arg(avatarFilePath).arg(size);
    QPixmap pixmap;
    if (QPixmapCache::find(key, &pixmap))
        return pixmap;

    // 加载原始图片
    QImage img;
    if (!avatarFilePath.isEmpty())
    {
        img = QImage(avatarFilePath);
        if (img.isNull())
        {
            img.load(AVATAR_SYSTEM);
        }
    }
    else
    {
        img = QImage(AVATAR_SYSTEM);
    }

    // 高 DPI 适配
    qreal dpr = qApp->devicePixelRatio(); // 或 painter->device()->devicePixelRatioF()
    QSize targetSize(size * dpr, size * dpr);

    // 缩放到目标大小
    img = img.scaled(targetSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    // 创建透明背景 QPixmap
    QPixmap rounded(targetSize);
    rounded.fill(Qt::transparent);

    // 圆角裁剪
    QPainter p(&rounded);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPainterPath path;
    path.addEllipse(0, 0, targetSize.width(), targetSize.height());
    p.setClipPath(path);
    p.drawImage(0, 0, img);
    p.end();

    // 存入缓存
    QPixmapCache::insert(key, rounded);

    return rounded;
}

// CMessageListWidget
CMessageListWidget::CMessageListWidget(QWidget *parent)
    : QListView(parent)
{
    m_model = new CMessageListModel(this);
    m_delegate = new CMessageDelegate(this);

    setModel(m_model);
    setItemDelegate(m_delegate);
    setSelectionMode(QAbstractItemView::NoSelection);   // 不可选择
    setUniformItemSizes(false);                         // 非统一大小
    setEditTriggers(QAbstractItemView::NoEditTriggers); // 不可编辑
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // NOTE 使用虚拟化列表可以实现多消息不卡顿
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); // Smooth scroll
}

void CMessageListWidget::resizeEvent(QResizeEvent *event)
{
    QListView::resizeEvent(event);
    // 清空缓存
    auto model = qobject_cast<CMessageListModel *>(this->model());
    if (model)
        model->clearCachedSizes();
    doItemsLayout();
}

void CMessageListWidget::addMessage(const CMessage &message)
{
    m_model->addMessage(message);
}

void CMessageListWidget::clearContext()
{
    m_model->addMessage(CMessage(DEFAULT_CONTENT_CLEAR_CONTEXT, Message::SYSTEM, getCurrentDateTime()));
}

void CMessageListWidget::clearAllMessage()
{
    m_model->clearAllMessage();
}