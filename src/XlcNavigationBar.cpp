#include "XlcNavigationBar.h"
#include <QVBoxLayout>
#include "Logger.hpp"
#include "ColorRepository.h"

/**
 * XlcNavigationTreeView
 */
XlcNavigationTreeView::XlcNavigationTreeView(QWidget *parent)
    : QTreeView(parent)
{
    // 隐藏表头
    setHeaderHidden(true);
    // 取消为顶层节点绘制展开/折叠箭头和连接线
    setRootIsDecorated(false);
    // 去除边框
    setFrameShape(QFrame::NoFrame);

    // 设置调色板
    setAutoFillBackground(true);
    QPalette palette = this->palette();
    palette.setBrush(QPalette::Base, ColorRepository::windowBackgroundColor());
    setPalette(palette);
}

bool XlcNavigationTreeView::isAncestorOf(const QModelIndex &ancestor, const QModelIndex &child) const
{
    if (!ancestor.isValid() || !child.isValid() || ancestor == child)
        return false;
    QModelIndex currentParent = child.parent();
    while (currentParent.isValid())
    {
        if (currentParent == ancestor)
            return true;
        currentParent = currentParent.parent();
    }
    return false;
}

void XlcNavigationTreeView::mousePressEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    if (!index.isValid())
    {
        return QTreeView::mousePressEvent(event);
    }

    // 叶子节点
    if (model()->rowCount(index) == 0)
    {
        // 叶子节点更新选中节点
        m_modelIndexSelectedLeaf = index;
        QTreeView::mousePressEvent(event);
        return;
    }

    // 非叶子节点
    isExpanded(index) ? collapse(index) : expand(index);
    // 选中项被折叠/打开
    if (isAncestorOf(index, m_modelIndexSelectedLeaf))
    {
        if (!isExpanded(index))
            // 折叠 - 选中父节点
            setCurrentIndex(index);
        else
            // 展开 - 选中原本节点
            setCurrentIndex(m_modelIndexSelectedLeaf);
    }
    // 拦截信号
    event->accept();
}

void XlcNavigationTreeView::changeEvent(QEvent *event)
{
    // 响应主题切换
    if (event->type() == QEvent::PaletteChange)
    {
        QPalette palette = this->palette();
        palette.setBrush(QPalette::Base, ColorRepository::windowBackgroundColor());
        setPalette(palette);
    }
    QTreeView::changeEvent(event);
}

void XlcNavigationTreeView::paintEvent(QPaintEvent *event)
{
    QTreeView::paintEvent(event);
    QPainter painter(viewport());
    painter.setPen(ColorRepository::basicBorderColor());
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(viewport()->rect().adjusted(1,1,-1,-1), 5, 5);
}

/**
 * XlcNavigationNode
 */
XlcNavigationNode::XlcNavigationNode(const QString &title, const QChar &iconfont, const QString &targetId, XlcNavigationNode *parent)
    : m_title(title), m_targetId(targetId), m_iconfont(iconfont), m_parentNode(parent)
{
}

XlcNavigationNode::~XlcNavigationNode()
{
    qDeleteAll(m_childNodes);
}

void XlcNavigationNode::appendChild(XlcNavigationNode *child)
{
    m_childNodes.append(child);
}

XlcNavigationNode *XlcNavigationNode::child(int row)
{
    if (row < 0 || row >= m_childNodes.size())
        return nullptr;
    return m_childNodes.at(row);
}

int XlcNavigationNode::childCount() const
{
    return m_childNodes.count();
}

QString XlcNavigationNode::title() const
{
    return m_title;
}

QString XlcNavigationNode::targetId() const
{
    return m_targetId;
}

QChar XlcNavigationNode::iconfont() const
{
    return m_iconfont;
}

int XlcNavigationNode::row() const
{
    if (m_parentNode)
        return m_parentNode->m_childNodes.indexOf(const_cast<XlcNavigationNode *>(this));
    return 0;
}

XlcNavigationNode *XlcNavigationNode::parentNode()
{
    return m_parentNode;
}

/**
 * XlcNavigationModel
 */
XlcNavigationModel::XlcNavigationModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    // 初始化根节点
    m_rootNode = new XlcNavigationNode(QString(), QChar(), QString(), nullptr);
}

XlcNavigationModel::~XlcNavigationModel()
{
    delete m_rootNode;
}

void XlcNavigationModel::addNavigationNode(const QString &title, const QChar &iconfont, const QString &targetId, const QModelIndex &parent)
{
    XlcNavigationNode *nodeParent = m_rootNode;
    if (parent.isValid())
        nodeParent = getNode(parent);
    // 尾插
    beginInsertRows(parent, nodeParent->childCount(), nodeParent->childCount());
    XlcNavigationNode *nodeNew = new XlcNavigationNode(title, iconfont, targetId, nodeParent);
    nodeParent->appendChild(nodeNew);
    endInsertRows();
}

QModelIndex XlcNavigationModel::findIndexByTitle(const QString &title, const QModelIndex &startParent) const
{
    // 从给定的父索引开始查找，如果 startParent 无效，则从根节点的子项开始
    return recursiveFind(title, startParent);
}

QVariant XlcNavigationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // 默认委托需要
    if (role == Qt::DisplayRole)
        return getNode(index)->title();

    switch (role)
    {
    case XlcNavigationNode::Role::Title:
        return getNode(index)->title();
    case XlcNavigationNode::Role::TargetId:
        return getNode(index)->targetId();
    case XlcNavigationNode::Role::Iconfont:
        return getNode(index)->iconfont();
    default:
        return QVariant();
    }
}

QModelIndex XlcNavigationModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    XlcNavigationNode *parentNode = getNode(parent);
    XlcNavigationNode *childNode = parentNode->child(row);

    if (childNode)
        return createIndex(row, column, childNode); // 使用 XlcNavigationNode 指针作为内部指针

    return QModelIndex();
}

QModelIndex XlcNavigationModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    XlcNavigationNode *childItem = getNode(index);
    XlcNavigationNode *parentItem = childItem->parentNode();

    if (parentItem == m_rootNode)
        return QModelIndex(); // 如果父项是根项，则返回无效索引

    return createIndex(parentItem->row(), 0, parentItem);
}

int XlcNavigationModel::rowCount(const QModelIndex &parent) const
{
    XlcNavigationNode *parentItem = getNode(parent);
    return parentItem->childCount();
}

int XlcNavigationModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

Qt::ItemFlags XlcNavigationModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return QAbstractItemModel::flags(index);
}

XlcNavigationNode *XlcNavigationModel::getNode(const QModelIndex &index) const
{
    if (index.isValid())
    {
        // 从 QModelIndex 中获取存储的内部指针
        XlcNavigationNode *node = static_cast<XlcNavigationNode *>(index.internalPointer());
        if (node)
            return node;
    }
    return m_rootNode;
}

QModelIndex XlcNavigationModel::recursiveFind(const QString &title, const QModelIndex &parentIndex) const
{
    // 遍历父节点下的所有子行
    for (int row = 0; row < rowCount(parentIndex); ++row)
    {
        // 获取当前行的 QModelIndex
        QModelIndex currentIndex = index(row, 0, parentIndex);

        // 检查当前节点的标题是否匹配
        QString currentTitle = data(currentIndex, XlcNavigationNode::Role::Title).toString();
        if (currentTitle == title)
        {
            return currentIndex; // 找到匹配项，立即返回
        }

        // 递归检查当前节点的子节点
        if (rowCount(currentIndex) > 0)
        {
            QModelIndex result = recursiveFind(title, currentIndex);
            if (result.isValid())
            {
                return result; // 在子树中找到匹配项，立即返回
            }
        }
    }

    return QModelIndex();
}

/**
 * XlcNavigationDelegate
 */
XlcNavigationDelegate::XlcNavigationDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void XlcNavigationDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    /**
     * 绘制背景
     */
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);
    if (option.state & QStyle::State_Selected)
    {
        // 选中且按压
        if (option.state & QStyle::State_Sunken)
        {
            painter->setBrush(ColorRepository::navigationSelectedAndPressedBackgroundColor());
        }
        // 选中且悬停
        else if (option.state & QStyle::State_MouseOver)
        {
            painter->setBrush(ColorRepository::navigationItemSelectedAndHoveredBackgroundColor());
        }
        // 选中
        else
        {
            painter->setBrush(ColorRepository::navigationItemSelectedBackgroundColor());
        }
    }
    // 悬停
    else if (option.state & QStyle::State_MouseOver)
    {
        painter->setBrush(ColorRepository::navigationItemHoveredBackgroundColor());
    }
    // 按压
    else if (option.state & QStyle::State_Sunken)
    {
        painter->setBrush(ColorRepository::navigationPressedBackgroundColor());
    }
    // 默认
    else
    {
        painter->setBrush(ColorRepository::navigationItemBackgroundColor());
    }
    painter->drawRoundedRect(option.rect.adjusted(0, SPACING_TOP, 0, -SPACING_TOP), RADIUS, RADIUS);
    painter->restore();

    /**
     * 绘制选中标记
     */
    if (option.state & QStyle::State_Selected)
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(Qt::NoPen);
        painter->setBrush(ColorRepository::primaryNormal());
        painter->drawRoundedRect(QRectF(option.rect.x() + OFFSET_MARK_X,
                                        option.rect.y() + SPACING_TOP + PADDING_MARK_TOP,
                                        WIDTH_MARK,
                                        option.rect.height() - SPACING_TOP - PADDING_MARK_TOP - PADDING_MARK_BOTTOM - PADDING_VERTICAL),
                                 RADIUS_MARK,
                                 RADIUS_MARK);
        painter->restore();
    }

    /**
     * 绘制图标
     */
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QChar iconfont = index.data(XlcNavigationNode::Role::Iconfont).toChar();
    QFont fontIconfont(ICONFONT_NAME);
    fontIconfont.setPixelSize(ICONFONT_SIZE);
    painter->setFont(fontIconfont);
    painter->setPen(ColorRepository::basicTextColor());
    QFontMetrics fmIconfont(fontIconfont);
    int sizeIcon = fmIconfont.horizontalAdvance(iconfont);
    QRect rectIconOriginal = option.widget->style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &option, option.widget);
    QRect rectIcon = QRect(rectIconOriginal.x() + MARGIN_INDICATOR_HORIZONTIAL,
                           rectIconOriginal.y() + (rectIconOriginal.height() - sizeIcon) / 2,
                           sizeIcon,
                           sizeIcon);
    painter->drawText(rectIcon, iconfont);
    painter->restore();

    /**
     * 绘制展开/折叠指示器
     */
    QRect rectIndicator;
    QAbstractItemModel *model = const_cast<QAbstractItemModel *>(index.model());
    if (!model || model->rowCount(index) != 0)
    {
        QChar iconfontArrow = ICONFONT_AngleDown;
        int size = fmIconfont.horizontalAdvance(iconfontArrow);
        rectIndicator = QRect(option.rect.x() + option.rect.width() - MARGIN_INDICATOR_HORIZONTIAL - size,
                              option.rect.y() + (option.rect.height() - size) / 2,
                              size,
                              size);
        if (option.state & QStyle::State_Open)
            iconfontArrow = ICONFONT_AngleUp;
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setFont(fontIconfont);
        painter->setPen(ColorRepository::basicTextColor());
        painter->drawText(rectIndicator, Qt::AlignCenter, iconfontArrow);
        painter->restore();
    }

    /**
     * 绘制文本
     */
    QString text = index.data(XlcNavigationNode::Role::Title).toString();
    if (text.isEmpty())
        return;
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QRect rectText = option.widget->style()->subElementRect(QStyle::SE_ItemViewItemText, &option, option.widget);
    QRect rectDrawText = rectText.adjusted(rectIcon.width() + SPACING_ICON_TO_TEXT, 0, 0, 0);
    // 非叶子节点加上指示器宽度
    if (model->rowCount(index) != 0)
        rectDrawText.adjust(0, 0, -MARGIN_INDICATOR_HORIZONTIAL - rectIndicator.width(), 0);
    // 省略文本
    text = option.fontMetrics.elidedText(text, Qt::TextElideMode::ElideRight, rectDrawText.width());
    QColor textColor;
    if (option.state & QStyle::State_Sunken)
        textColor = ColorRepository::pressedTextColor();
    else if (option.state & QStyle::State_MouseOver)
        textColor = ColorRepository::hoverTextColor();
    else
        textColor = ColorRepository::basicTextColor();
    painter->setPen(QColor(textColor));
    painter->drawText(rectDrawText, Qt::AlignVCenter, text);
    painter->restore();
}

QSize XlcNavigationDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize sizeOriginal = QStyledItemDelegate::sizeHint(option, index);
    return QSize(sizeOriginal.width(), sizeOriginal.height() + PADDING_VERTICAL * 2);
}

/**
 * XlcNavigationBar
 */
XlcNavigationBar::XlcNavigationBar(BaseWidget *parent)
    : BaseWidget(parent)
{
    initUI();
}

void XlcNavigationBar::addNavigationNode(const QString &title, const QChar &iconfont, const QString &targetId, const QString &parentTitle)
{
    QModelIndex parentIndex = QModelIndex();
    if (!parentTitle.isEmpty())
    {
        parentIndex = m_navigationModel->findIndexByTitle(parentTitle);
        if (!parentIndex.isValid())
        {
            XLC_LOG_WARN("Add navigation node failed (title={}, targetId={}, parentTitle={}): check if the parent title exists", title, targetId, parentTitle);
            return;
        }
    }
    m_navigationModel->addNavigationNode(title, iconfont, targetId, parentIndex);
}

void XlcNavigationBar::setSelectedItem(const QString &title)
{
    QModelIndex targetIndex = m_navigationModel->findIndexByTitle(title);
    if (!targetIndex.isValid())
        return;
    m_treeView->setCurrentIndex(targetIndex);
}

void XlcNavigationBar::initWidget()
{
}

void XlcNavigationBar::initItems()
{
    // m_navigationModel
    m_navigationModel = new XlcNavigationModel(this);
    // m_treeView
    m_treeView = new XlcNavigationTreeView(this);
    m_treeView->setItemDelegate(new XlcNavigationDelegate(this));
    m_treeView->setModel(m_navigationModel);
    m_treeView->expandAll();
    // 连接选择模型的 currentChanged 信号
    QItemSelectionModel *selectionModel = m_treeView->selectionModel();
    if (selectionModel)
    {
        connect(selectionModel, &QItemSelectionModel::currentChanged, this, &XlcNavigationBar::slot_handleCurrentItemChanged);
    }
}

void XlcNavigationBar::initLayout()
{
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addWidget(m_treeView);
}

void XlcNavigationBar::slot_handleCurrentItemChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if (!current.isValid())
        return;

    // 检查该项是否是叶子节点
    if (m_treeView->model()->rowCount(current) == 0)
    {
        // 获取节点数据
        QString targetId = current.data(XlcNavigationNode::Role::TargetId).toString();
        if (m_treeView->isAncestorOf(previous, current))
        {
            // 还原事件，直接返回
            return;
        }
        emit sig_currentItemChanged(targetId);
    }
}