#include "AgentListWidget.h"
#include "ColorRepository.h"
#include "PainterHelper.h"

/**
 * AgentListModel
 */
AgentListModel::AgentListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int AgentListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_items.count();
}

QVariant AgentListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const AgentItem &item = m_items.at(index.row());
    if (role == Uuid)
        return item.uuid;
    if (role == Name)
        return item.name;
    if (role == ConversationCount)
        return item.conversationCount;

    return QVariant();
}

void AgentListModel::addItem(const QString &uuid, const QString &name, int conversationCount)
{
    beginInsertRows(QModelIndex(), m_items.count(), m_items.count());
    AgentItem item = {uuid, name, conversationCount};
    m_items.append(item);
    endInsertRows();
}

QModelIndex AgentListModel::findIndex(const QString &uuid)
{
    for (int i = 0; i < m_items.size(); ++i)
    {
        if (m_items.at(i).uuid == uuid)
        {
            return index(i);
        }
    }
    return QModelIndex();
}

void AgentListModel::clearData()
{
    if (m_items.isEmpty())
    {
        return;
    }

    int rowCount = m_items.count();
    beginRemoveRows(QModelIndex(), 0, rowCount - 1);
    m_items.clear();
    endRemoveRows();
}

/**
 * AgentDelegate
 */
AgentDelegate::AgentDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void AgentDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    /**
     * 绘制背景
     */
    QFont font = getGlobalFont();
    font.setPointSize(SIZE_FONT);
    QFontMetrics fm = QFontMetrics(font);
    int widthBackground = option.rect.width() - MARGIN * 2;
    int heightBackground = PADDING_VERTICAL + fm.height() + PADDING_VERTICAL;
    int xBackground = option.rect.x() + (option.rect.width() - widthBackground) / 2;
    int yBackground = option.rect.y() + (option.rect.height() - heightBackground) / 2;
    QRect rectBackground = QRect(xBackground, yBackground, widthBackground, heightBackground);
    int radius = heightBackground / 2;

    PainterHelper::drawBackground(painter, option, rectBackground, radius);

    /**
     * 获取数据
     */
    QString name = index.data(AgentListModel::Name).toString();
    int conversationCount = index.data(AgentListModel::ConversationCount).toInt();

    /**
     * 绘制对话计数
     */
    QRect rectBackgroundConversationCount = QRect();
    if (option.state & QStyle::State_Selected)
    {
        painter->save();
        // 绘制背景
        QFont fontConversationCount = getGlobalFont();
        fontConversationCount.setPointSize(FONT_SIZE_CONVERSATION_COUNT);
        QFontMetrics fmConversationCount(fontConversationCount);

        QRect rectConversationCount = fmConversationCount.boundingRect(QString::number(conversationCount));
        int hBackgroundConversationCount = PADDING_CONVERSATION_COUNT + rectConversationCount.height() + PADDING_CONVERSATION_COUNT;
        int wBackgroundConversationCount = qMax(hBackgroundConversationCount,
                                                PADDING_CONVERSATION_COUNT + fmConversationCount.horizontalAdvance(QString::number(conversationCount)) + PADDING_CONVERSATION_COUNT);
        int xBackgroundConversationCount = rectBackground.topRight().x() - wBackgroundConversationCount - PADDING_HORIZONTAL;
        int yBackgroundConversationCount = rectBackground.y() + (rectBackground.height() - hBackgroundConversationCount) / 2;
        int radiusBackgroundConversationCount = hBackgroundConversationCount / 2;
        rectBackgroundConversationCount = QRect(xBackgroundConversationCount,
                                                yBackgroundConversationCount,
                                                wBackgroundConversationCount,
                                                hBackgroundConversationCount);
        painter->setPen(ColorRepository::listSelectedAndHoveredOutlineColor());
        painter->setBrush(ColorRepository::baseBackgroundColor());
        painter->drawRoundedRect(rectBackgroundConversationCount, radiusBackgroundConversationCount, radiusBackgroundConversationCount);
        // 绘制计数
        QRect rectDrawConversationCount = rectBackgroundConversationCount.adjusted(PADDING_CONVERSATION_COUNT,
                                                                                   PADDING_CONVERSATION_COUNT,
                                                                                   -PADDING_CONVERSATION_COUNT,
                                                                                   -PADDING_CONVERSATION_COUNT);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(ColorRepository::basicText());
        painter->setFont(fontConversationCount);
        painter->drawText(rectBackgroundConversationCount, Qt::AlignCenter, QString::number(conversationCount));

        painter->restore();
    }

    /**
     * 绘制名字
     */
    QRect rectText = QRect(rectBackground.adjusted(PADDING_HORIZONTAL,
                                                   PADDING_VERTICAL,
                                                   -PADDING_HORIZONTAL - rectBackgroundConversationCount.width() - SPACING_NAME_TO_CONVERSATION_COUNT,
                                                   -PADDING_VERTICAL));
    QString elidedName = fm.elidedText(name, Qt::ElideRight, rectText.width());
    painter->setPen(ColorRepository::basicText());
    painter->drawText(rectText, elidedName);

    painter->restore();
}

QSize AgentDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QFont font = getGlobalFont();
    font.setPointSize(SIZE_FONT);
    QFontMetrics fm = QFontMetrics(font);
    int height = MARGIN + PADDING_VERTICAL + fm.height() + PADDING_VERTICAL;
    return QSize(option.rect.width(), height);
}

/**
 * AgentListWidget
 */
AgentListWidget::AgentListWidget(QWidget *parent)
    : QListView(parent)
{
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    m_model = new AgentListModel(this);
    m_delegate = new AgentDelegate(this);
    setModel(m_model);
    setItemDelegate(m_delegate);

    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this,
            [this](const QItemSelection &selected, const QItemSelection &deselected)
            {
                QModelIndexList selectedIndexes = selected.indexes();
                if (selectedIndexes.isEmpty())
                    return;

                QModelIndex firstSelected = selectedIndexes.first();
                if (firstSelected.isValid())
                {
                    Q_EMIT itemChanged(firstSelected.data(AgentListModel::Uuid).toString());
                }
            });
}

void AgentListWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter(viewport());
    painter.fillRect(rect(), ColorRepository::baseBackgroundColor());
    QListView::paintEvent(e);
}

void AgentListWidget::addItem(const QString &uuid, const QString &name, int conversationCount)
{
    m_model->addItem(uuid, name, conversationCount);
}

void AgentListWidget::setCurrentAgent(const QString &uuid)
{
    QModelIndex indexToSelect = m_model->findIndex(uuid);
    if (indexToSelect.isValid())
    {
        QItemSelectionModel *selectionModel = this->selectionModel();
        if (selectionModel)
        {
            selectionModel->select(indexToSelect, QItemSelectionModel::ClearAndSelect);
            setCurrentIndex(indexToSelect);
            scrollTo(indexToSelect);
        }
    }
}

void AgentListWidget::selectFirstAgent()
{
    QModelIndex firstIndex = m_model->index(0, 0);
    if (firstIndex.isValid())
    {
        selectionModel()->setCurrentIndex(firstIndex, QItemSelectionModel::ClearAndSelect);
        QItemSelectionModel *selectionModel = this->selectionModel();
        if (selectionModel)
        {
            selectionModel->select(firstIndex, QItemSelectionModel::ClearAndSelect);
            setCurrentIndex(firstIndex);
            scrollTo(firstIndex);
        }
    }
}

QString AgentListWidget::currentAgentUuid()
{
    QItemSelectionModel *selectionModel = this->selectionModel();
    if (!selectionModel)
    {
        return QString();
    }

    QModelIndex currentIndex = selectionModel->currentIndex();
    if (!currentIndex.isValid())
    {
        return QString();
    }
    return currentIndex.data(AgentListModel::Uuid).toString();
}

bool AgentListWidget::hasAgentSelected()
{
    return this->selectionModel()->hasSelection();
}

void AgentListWidget::clear()
{
    m_model->clearData();
}