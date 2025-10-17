#include "NavigationBar.h"
#include <QPainter>
#include <QSvgRenderer>
#include "Logger.hpp"
#include "global.h"
#include "ColorRepository.h"
#include "PainterHelper.h"

/**
 * NavigationItemListModel
 */
NavigationItemListModel::NavigationItemListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int NavigationItemListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_items.count();
}

QVariant NavigationItemListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const NavigationItem &item = m_items.at(index.row());
    if (role == Text)
        return item.text;
    if (role == IconFilePath)
        return item.iconFilePath;

    return QVariant();
}

void NavigationItemListModel::addItem(const QString &text, const QString &iconFilePath)
{
    // 插入数据前通知 View
    beginInsertRows(QModelIndex(), m_items.count(), m_items.count());

    NavigationItem item = {text, iconFilePath};
    m_items.append(item);

    // 插入数据后通知 View
    endInsertRows();
}

QModelIndex NavigationItemListModel::findIndexByText(const QString &text) const
{
    for (int row = 0; row < m_items.count(); ++row)
    {
        if (m_items.at(row).text == text)
        {
            return index(row, 0);
        }
    }
    return QModelIndex();
}

/**
 * NavigationItemDelegate
 */
NavigationItemDelegate::NavigationItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void NavigationItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect rect = option.rect;
    /**
     * 绘制背景
     */
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    int backgroundWidth = rect.width() - MARGIN * 2;
    QRect rectBackground = QRect(rect.x() + (rect.width() - backgroundWidth) / 2, rect.y() + MARGIN, backgroundWidth, backgroundWidth);

    PainterHelper::drawBackground(painter, option, rectBackground, RADIUS, ColorRepository::windowBackground());

    /**
     * 获取数据
     */
    QString text = index.data(NavigationItemListModel::Text).toString();
    QString iconFilePath = index.data(NavigationItemListModel::IconFilePath).toString();

    /**
     * 绘制图标
     */
    QRect rectIcon = rectBackground.adjusted(PADDING, PADDING, -PADDING, -PADDING);
    QSvgRenderer renderer(iconFilePath);
    if (!renderer.isValid())
    {
        renderer.load(DEFAULT_ICON);
    }
    renderer.render(painter, rectIcon);

    /**
     * 绘制文本
     */
    QFont font = getGlobalFont();
    font.setPointSize(FONT_SIZE);
    QFontMetrics fm = QFontMetrics(font);
    int textWidth = fm.horizontalAdvance(text);
    QRect rectText = fm.boundingRect(0, 0, rectBackground.width(), 0, Qt::TextWrapAnywhere, text);
    QRect rectDrawText = QRect((option.rect.width() - rectText.width()) / 2,
                               rectBackground.y() + rectBackground.height() + SPACING_ICON_TO_TEXT,
                               rectText.width(),
                               rectText.height());

    painter->setPen(ColorRepository::basicText());
    painter->drawText(rectDrawText, Qt::TextWrapAnywhere, text);
    painter->restore();
}

QSize NavigationItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int heightBackground = option.rect.width() - MARGIN * 2;

    QFont font = getGlobalFont();
    font.setPointSize(FONT_SIZE);
    QFontMetrics fm = QFontMetrics(font);
    QString text = index.data(NavigationItemListModel::Text).toString();
    QRect rectText = fm.boundingRect(0, 0, option.rect.width(), 0, Qt::TextWrapAnywhere, text);

    int height = MARGIN + heightBackground + SPACING_ICON_TO_TEXT + rectText.height();
    return QSize(option.rect.width(), height);
}

/**
 * NavigationBar
 */
NavigationBar::NavigationBar(QWidget *parent)
    : QListView(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    m_model = new NavigationItemListModel(this);
    m_delegate = new NavigationItemDelegate(this);
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
                    Q_EMIT indexChanged(firstSelected.row());
                }
            });
}

void NavigationBar::paintEvent(QPaintEvent *e)
{
    QPainter painter(viewport());
    painter.fillRect(rect(), ColorRepository::windowBackground());
    QListView::paintEvent(e);
}

// void NavigationBar::addItem(const QString &text, const QString &iconFilePath, bool selectable)
void NavigationBar::addItem(const QString &text, const QString &iconFilePath)
{
    // m_model->addItem(text, iconFilePath, selectable);
    m_model->addItem(text, iconFilePath);
}

void NavigationBar::setCurrentText(const QString &text)
{
    QModelIndex indexToSelect = m_model->findIndexByText(text);

    if (indexToSelect.isValid())
    {
        QItemSelectionModel *selectionModel = this->selectionModel();
        if (selectionModel)
        {
            selectionModel->select(indexToSelect, QItemSelectionModel::ClearAndSelect);
            // 设置当前项，确保键盘焦点和视觉高亮到位
            setCurrentIndex(indexToSelect);
            // 确保新选中的项在视图中可见
            scrollTo(indexToSelect);
        }
    }
}