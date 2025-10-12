#ifndef BASESETTINGSPAGE_H
#define BASESETTINGSPAGE_H

#include "BaseWidget.hpp"
#include "DataManager.h"
#include "Logger.hpp"
#include "EventBus.h"
#include <QListWidget>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QJsonObject>
#include <functional>
#include "ToastManager.h"

template <typename T, typename TWidget, typename TDialog>
class BaseSettingsPage : public BaseWidget
{
public:
    // 使用 std::function 抽象数据操作
    using GetAllFunc = std::function<QList<std::shared_ptr<T>>()>;
    using GetOneFunc = std::function<std::shared_ptr<T>(const QString &)>;
    using AddFunc = std::function<void(std::shared_ptr<T>)>;
    using UpdateFunc = std::function<void(std::shared_ptr<T>)>;
    using RemoveFunc = std::function<void(const QString &)>;
    using GetNameFunc = std::function<QString(const std::shared_ptr<T> &)>;

    /**
     * @brief 实体编辑页面构造函数
     *
     *
     * @param entityName 实体名称
     * @param fnGetAll 获取所有实体的函数指针
     * @param fnGetOne 获取单个实体的函数指针
     * @param fnAdd 添加新实体的函数指针
     * @param fnUpdate 更新实体的函数指针
     * @param fnRemove 删除实体的函数指针
     * @param fnGetName 获取实体名称的函数指针
     * @param updateEventState 实体更新事件的信号类型
     * @param parent 父窗口指针
     */
    explicit BaseSettingsPage(const QString &entityName,
                              GetAllFunc fnGetAll,
                              GetOneFunc fnGetOne,
                              AddFunc fnAdd,
                              UpdateFunc fnUpdate,
                              RemoveFunc fnRemove,
                              GetNameFunc fnGetName,
                              EventBus::States updateEventState,
                              QWidget *parent = nullptr)
        : BaseWidget(parent),
          m_entityName(entityName),
          m_fnGetAll(fnGetAll),
          m_fnGetOne(fnGetOne),
          m_fnAdd(fnAdd),
          m_fnUpdate(fnUpdate),
          m_fnRemove(fnRemove),
          m_fnGetName(fnGetName),
          m_updateEventState(updateEventState)
    {
        initUI();
    }

    virtual ~BaseSettingsPage() = default;

protected:
    void initItems() override
    {
        // 创建通用UI组件
        m_listWidget = new QListWidget(this);
        m_detailWidget = new TWidget(this);
        m_pushButtonAdd = new QPushButton("添加", this);
        m_pushButtonReset = new QPushButton("重置", this);
        m_pushButtonRemove = new QPushButton("删除", this);
        m_pushButtonSave = new QPushButton("保存", this);

        // 连接通用信号槽
        connect(m_listWidget, &QListWidget::itemClicked, this, &BaseSettingsPage::slot_onListWidgetItemClicked);
        connect(m_pushButtonAdd, &QPushButton::clicked, this, &BaseSettingsPage::slot_onPushButtonClickedAdd);
        connect(m_pushButtonReset, &QPushButton::clicked, this, &BaseSettingsPage::slot_onPushButtonClickedReset);
        connect(m_pushButtonRemove, &QPushButton::clicked, this, &BaseSettingsPage::slot_onPushButtonClickedRemove);
        connect(m_pushButtonSave, &QPushButton::clicked, this, &BaseSettingsPage::slot_onPushButtonClickedSave);
    }

    void initLayout() override
    {
        QHBoxLayout *hLayoutButtons = new QHBoxLayout();
        hLayoutButtons->setContentsMargins(0, 0, 0, 0);
        hLayoutButtons->addWidget(m_pushButtonAdd);
        hLayoutButtons->addStretch();
        hLayoutButtons->addWidget(m_pushButtonReset);
        hLayoutButtons->addWidget(m_pushButtonRemove);
        hLayoutButtons->addWidget(m_pushButtonSave);

        QWidget *widgetOption = new QWidget(this);
        QVBoxLayout *vLayoutOption = new QVBoxLayout(widgetOption);
        vLayoutOption->addWidget(m_detailWidget);
        vLayoutOption->addStretch();
        vLayoutOption->addLayout(hLayoutButtons);

        QSplitter *splitter = new QSplitter(this);
        splitter->setChildrenCollapsible(false);
        splitter->addWidget(m_listWidget);
        splitter->addWidget(widgetOption);
        splitter->setStretchFactor(0, 2);
        splitter->setStretchFactor(1, 8);

        QVBoxLayout *vLayout = new QVBoxLayout(this);
        vLayout->setContentsMargins(0, 0, 0, 0);
        vLayout->addWidget(splitter);
    }

    // 子类必须实现此方法以连接到特定的 DataManager 信号
    virtual void connectDataManagerDataLoadedSignals() = 0;

public Q_SLOTS:
    void slot_onDataLoaded(bool success)
    {
        if (!success)
            return;

        QList<std::shared_ptr<T>> items = m_fnGetAll();
        m_listWidget->clear();
        for (const std::shared_ptr<T> &item : items)
        {
            QListWidgetItem *listItem = new QListWidgetItem(m_fnGetName(item), m_listWidget);
            listItem->setData(Qt::UserRole, QVariant::fromValue<QString>(item->uuid));
            m_listWidget->addItem(listItem);
        }
        m_listWidget->sortItems();

        if (m_listWidget->count() > 0 && m_listWidget->currentItem() == nullptr)
        {
            m_listWidget->setCurrentRow(0);
            QListWidgetItem *selectedItem = m_listWidget->currentItem();
            if (selectedItem)
            {
                showInfo(selectedItem->data(Qt::UserRole).value<QString>());
            }
        }
    }

private Q_SLOTS:
    void slot_onListWidgetItemClicked(QListWidgetItem *item)
    {
        const QString &uuid = item->data(Qt::UserRole).value<QString>();
        XLC_LOG_TRACE("{} selected (name={}, uuid={})", m_entityName, item->text(), uuid);
        showInfo(uuid);
    }

    void slot_onPushButtonClickedAdd()
    {
        XLC_LOG_TRACE("Attempting to add {}", m_entityName);
        TDialog *dialog = new TDialog(this);
        connect(dialog, &QDialog::finished, this,
                [this, dialog](int result)
                {
                    if (result == QDialog::Accepted)
                    {
                        std::shared_ptr<T> newItem = dialog->getFormData();
                        XLC_LOG_TRACE("Adding new {} (uuid={}, name={})", m_entityName, newItem->uuid, m_fnGetName(newItem));
                        m_fnAdd(newItem);

                        QListWidgetItem *listItem = new QListWidgetItem();
                        listItem->setText(m_fnGetName(newItem));
                        listItem->setData(Qt::UserRole, QVariant::fromValue(newItem->uuid));
                        m_listWidget->addItem(listItem);

                        m_listWidget->setCurrentItem(listItem);
                        m_detailWidget->updateFormData(newItem);

                        publishUpdate();
                    }
                });
        dialog->exec();
    }

    void slot_onPushButtonClickedReset()
    {
        std::shared_ptr<T> item = m_fnGetOne(m_detailWidget->getUuid());
        if (!item)
            return;
        m_detailWidget->updateFormData(item);
        XLC_LOG_DEBUG("Reloading {} widget (UUID={})", m_entityName, m_detailWidget->getUuid());
    }

    void slot_onPushButtonClickedRemove()
    {
        const QString uuid = m_detailWidget->getUuid();
        if (uuid.isEmpty())
            return;

        if (QMessageBox::Yes == QMessageBox::question(this,
                                                      "确认删除", QString("是否确认删除 %1: %2？").arg(m_entityName).arg(uuid),
                                                      QMessageBox::Yes | QMessageBox::No,
                                                      QMessageBox::Yes))
        {
            m_fnRemove(uuid);

            QListWidgetItem *currentItem = m_listWidget->currentItem();
            if (currentItem)
            {
                delete m_listWidget->takeItem(m_listWidget->row(currentItem));
            }

            if (m_listWidget->count() > 0)
            {
                m_listWidget->setCurrentRow(0);
                QListWidgetItem *selectedItem = m_listWidget->currentItem();
                if (selectedItem)
                {
                    showInfo(selectedItem->data(Qt::UserRole).value<QString>());
                }
            }
            else
            {
                m_detailWidget->clearFormData();
            }

            publishUpdate();
        }
    }

    void slot_onPushButtonClickedSave()
    {
        const QString uuid = m_detailWidget->getUuid();
        XLC_LOG_DEBUG("Saving/Updating {} (uuid={})", m_entityName, uuid);
        std::shared_ptr<T> currentData = m_detailWidget->getCurrentData();
        m_fnUpdate(currentData);

        QListWidgetItem *selectedItem = m_listWidget->currentItem();
        if (selectedItem && selectedItem->data(Qt::UserRole).toString() == uuid)
        {
            selectedItem->setText(m_fnGetName(currentData));
        }

        publishUpdate();
    }

private:
    void showInfo(const QString &uuid)
    {
        std::shared_ptr<T> item = m_fnGetOne(uuid);
        if (!item)
        {
            XLC_LOG_WARN("{} not found (uuid={})", m_entityName, uuid);
            ToastManager::showMessage(Toast::Type::Warning, QString("%1 not found (uuid=%2)").arg(m_entityName).arg(uuid));
            return;
        }
        m_detailWidget->updateFormData(item);
    }

    void publishUpdate()
    {
        QJsonObject jsonObj;
        jsonObj["id"] = static_cast<int>(m_updateEventState);
        EventBus::getInstance()->publish(EventBus::EventType::StateChanged, QVariant(jsonObj));
    }

private:
    // UI组件
    QListWidget *m_listWidget;
    TWidget *m_detailWidget;
    QPushButton *m_pushButtonAdd;
    QPushButton *m_pushButtonRemove;
    QPushButton *m_pushButtonReset;
    QPushButton *m_pushButtonSave;

    // 数据操作函数
    QString m_entityName;
    GetAllFunc m_fnGetAll;
    GetOneFunc m_fnGetOne;
    AddFunc m_fnAdd;
    UpdateFunc m_fnUpdate;
    RemoveFunc m_fnRemove;
    GetNameFunc m_fnGetName;
    EventBus::States m_updateEventState;
};

#endif // BASESETTINGSPAGE_H