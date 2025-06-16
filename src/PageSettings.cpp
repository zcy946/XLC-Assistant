#include "PageSettings.h"
#include <QSplitter>
#include "Logger.hpp"
#include <QVBoxLayout>
#include <QLabel>
#include "global.h"
#include <QGridLayout>
#include <QSpacerItem>

// PageSettings
PageSettings::PageSettings(QWidget *parent)
    : BaseWidget(parent)
{
    initUI();
    // pageSettingsAgent
    addPage("助手设置", new PageSettingsAgent(this));
    // pageSettingsMcp
    addPage("MCP 服务器", new PageSettingsMcp(this));
    // pageSettingData
    addPage("存储设置", new PageSettingsData(this));
    // pageAbout
    addPage("关于", new PageAbout(this));
#ifdef QT_DEBUG
    for (int i = 0; i < 30; ++i)
    {
        QString nameSetting = "设置测试实例" + QString::number(i + 1);
        QListWidgetItem *itemSetting = new QListWidgetItem();
        itemSetting->setText(nameSetting);
        m_listWidget->addItem(itemSetting);
    }
#endif
}

void PageSettings::initWidget()
{
}

void PageSettings::initItems()
{
    // m_listWidget
    m_listWidget = new QListWidget(this);
    connect(m_listWidget, &QListWidget::currentTextChanged, this,
            [this](const QString &currentText)
            {
                const auto &it = m_pages.find(currentText);
                if (it == m_pages.end())
                {
                    LOG_WARN("不存在的设置项: {}", currentText);
                    return;
                }
                m_stackedWidget->setCurrentWidget(it.value());
                LOG_DEBUG("选中设置项: {}", currentText);
            });

    // m_stackedWidget
    m_stackedWidget = new QStackedWidget(this);
}

void PageSettings::initLayout()
{
    // splitter
    QSplitter *splitter = new QSplitter(this);
    splitter->setChildrenCollapsible(false);
    splitter->addWidget(m_listWidget);
    splitter->addWidget(m_stackedWidget);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 8);
    // vLayout
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addWidget(splitter);
}

void PageSettings::addPage(const QString &name, QWidget *page)
{
    m_listWidget->addItem(name);
    m_stackedWidget->addWidget(page);
    m_pages.insert(name, page);
}

// PageSettingsAgent
PageSettingsAgent::PageSettingsAgent(QWidget *parent)
    : BaseWidget(parent)
{
    initUI();
    connect(DataManager::getInstance(), &DataManager::sig_agentsLoaded, this, &PageSettingsAgent::slot_onAgentsOrMcpServersLoaded);
    connect(DataManager::getInstance(), &DataManager::sig_mcpServersLoaded, this, &PageSettingsAgent::slot_onAgentsOrMcpServersLoaded);
}

void PageSettingsAgent::initWidget()
{
}

void PageSettingsAgent::initItems()
{
    // m_listWidgetAgents
    m_listWidgetAgents = new QListWidget(this);
    connect(m_listWidgetAgents, &QListWidget::itemClicked, this, &PageSettingsAgent::slot_onListWidgetItemClicked);
    // m_widgetAgentInfo
    m_widgetAgentInfo = new WidgetAgentInfo(this);
}

void PageSettingsAgent::initLayout()
{
    // splitter
    QSplitter *splitter = new QSplitter(this);
    splitter->setChildrenCollapsible(false);
    splitter->addWidget(m_listWidgetAgents);
    splitter->addWidget(m_widgetAgentInfo);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 8);
    // vLayout
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addWidget(splitter);
}

void PageSettingsAgent::slot_onListWidgetItemClicked(QListWidgetItem *item)
{
    const QString &agentName = item->data(Qt::DisplayRole).value<QString>();
    const QString &agentUuid = item->data(Qt::UserRole).value<QString>();
    LOG_DEBUG("选中agent: {} - {}", agentName, agentUuid);
    showAgentInfo(agentUuid);
}

void PageSettingsAgent::slot_onAgentsOrMcpServersLoaded(bool success)
{
    if (!success)
        return;
    QList<std::shared_ptr<Agent>> agents = DataManager::getInstance()->getAgents();
    QList<std::shared_ptr<McpServer>> mcpServers = DataManager::getInstance()->getMcpServers();
    if (agents.isEmpty() || mcpServers.isEmpty())
        return;
    for (const std::shared_ptr<Agent> &agent : agents)
    {
        QListWidgetItem *itemAgent = new QListWidgetItem(agent->name, m_listWidgetAgents);
        itemAgent->setData(Qt::UserRole, agent->uuid);
    }
    // 默认选中并展示第一项
    if (m_listWidgetAgents->currentItem() == nullptr)
    {
        m_listWidgetAgents->setCurrentRow(0);
        m_widgetAgentInfo->updateData(DataManager::getInstance()->getAgent(m_listWidgetAgents->currentItem()->data(Qt::UserRole).value<QString>()));
    }
}

void PageSettingsAgent::showAgentInfo(const QString &uuid)
{
    const std::shared_ptr<Agent> &agent = DataManager::getInstance()->getAgent(uuid);
    if (!agent)
    {
        LOG_WARN("不存在的agent: {}", uuid);
        return;
    }
    m_widgetAgentInfo->updateData(agent);
}

// WidgetAgentInfo
WidgetAgentInfo::WidgetAgentInfo(QWidget *parent)
{
    initUI();
}

void WidgetAgentInfo::initWidget()
{
}

void WidgetAgentInfo::initItems()
{
    // m_lineEditUuid
    m_lineEditUuid = new QLineEdit(this);
    m_lineEditUuid->setReadOnly(true);
    m_lineEditUuid->setPlaceholderText("UUID");
    // m_lineEditName
    m_lineEditName = new QLineEdit(this);
    m_lineEditName->setPlaceholderText("名称");
    // m_spinBoxChildren
    m_spinBoxChildren = new QSpinBox(this);
    // m_plainTextEditDescription
    m_plainTextEditDescription = new QPlainTextEdit(this);
    m_plainTextEditDescription->setPlaceholderText("描述");
    // m_lineEditModelName
    m_lineEditModelName = new QLineEdit(this);
    m_lineEditModelName->setPlaceholderText("模型名称");
    // m_spinBoxContext
    m_spinBoxContext = new QSpinBox(this);
    // m_doubleSpinBoxTemperature
    m_doubleSpinBoxTemperature = new QDoubleSpinBox(this);
    // m_doubleSpinBoxTopP
    m_doubleSpinBoxTopP = new QDoubleSpinBox(this);
    // m_spinBoxMaxTokens
    m_spinBoxMaxTokens = new QSpinBox(this);
    // m_plainTextEditSystemPrompt
    m_plainTextEditSystemPrompt = new QPlainTextEdit(this);
    m_plainTextEditSystemPrompt->setPlaceholderText("系统提示词");
    // m_listWidgetMcpServers
    m_listWidgetMcpServers = new QListWidget(this);
    // m_pushButtonReset
    m_pushButtonReset = new QPushButton("重置", this);
    connect(m_pushButtonReset, &QPushButton::clicked, this,
            []()
            {
                // TODO 重载
                LOG_DEBUG("重载agent设置");
            });
    // m_pushButtonSave
    m_pushButtonSave = new QPushButton("保存", this);
    connect(m_pushButtonSave, &QPushButton::clicked, this,
            []()
            {
                // TODO 保存
                LOG_DEBUG("保存agent设置");
            });
}

void WidgetAgentInfo::initLayout()
{
    // gLayout
    QGridLayout *gLayout = new QGridLayout(this);
    gLayout->addWidget(new QLabel("UUID", this), 0, 0);
    gLayout->addWidget(m_lineEditUuid, 0, 1);
    gLayout->addWidget(new QLabel("名称", this), 1, 0);
    gLayout->addWidget(m_lineEditName, 1, 1);
    gLayout->addWidget(new QLabel("实例数量", this), 2, 0);
    gLayout->addWidget(m_spinBoxChildren, 2, 1);
    gLayout->addWidget(new QLabel("描述", this), 3, 0);
    gLayout->addWidget(m_plainTextEditDescription, 3, 1);
    gLayout->addWidget(new QLabel("模型名称", this), 4, 0);
    gLayout->addWidget(m_lineEditModelName, 4, 1);
    gLayout->addWidget(new QLabel("上下文", this), 5, 0);
    gLayout->addWidget(m_spinBoxContext, 5, 1);
    gLayout->addWidget(new QLabel("模型温度", this), 6, 0);
    gLayout->addWidget(m_doubleSpinBoxTemperature, 6, 1);
    gLayout->addWidget(new QLabel("Top-p", this), 7, 0);
    gLayout->addWidget(m_doubleSpinBoxTopP, 7, 1);
    gLayout->addWidget(new QLabel("最大Token数", this), 8, 0);
    gLayout->addWidget(m_spinBoxMaxTokens, 8, 1);
    gLayout->addWidget(new QLabel("系统提示词", this), 9, 0);
    gLayout->addWidget(m_plainTextEditSystemPrompt, 9, 1);
    gLayout->addWidget(new QLabel("MCP服务器", this), 10, 0);
    gLayout->addWidget(m_listWidgetMcpServers, 10, 1);
    // hLayoutButtons
    QHBoxLayout *hLayoutButtons = new QHBoxLayout();
    hLayoutButtons->setContentsMargins(0, 0, 0, 0);
    hLayoutButtons->addStretch();
    hLayoutButtons->addWidget(m_pushButtonReset);
    hLayoutButtons->addWidget(m_pushButtonSave);
    gLayout->addLayout(hLayoutButtons, 11, 1);
}

void WidgetAgentInfo::updateData(std::shared_ptr<Agent> agent)
{
    m_lineEditUuid->setText(agent->uuid);
    m_lineEditName->setText(agent->name);
    m_spinBoxChildren->setValue(agent->children);
    m_plainTextEditDescription->setPlainText(agent->description);
    m_lineEditModelName->setText(agent->modelName);
    m_spinBoxContext->setValue(agent->context);
    m_doubleSpinBoxTemperature->setValue(agent->temperature);
    m_doubleSpinBoxTopP->setValue(agent->topP);
    m_spinBoxMaxTokens->setValue(agent->maxTokens);
    m_plainTextEditSystemPrompt->setPlainText(agent->systemPrompt);
    for (const QString &uuid : agent->mcpServers)
    {
        const std::shared_ptr<McpServer> &mcpServer = DataManager::getInstance()->getMcpServer(uuid);
        if (!mcpServer)
        {
            LOG_WARN("不存在的mcp服务器: {}", uuid);
            continue;
        }
        m_listWidgetMcpServers->addItem(mcpServer->name);
    }
}

// PageSettingsMcp
PageSettingsMcp::PageSettingsMcp(QWidget *parent)
    : BaseWidget(parent)
{
    initUI();
}

void PageSettingsMcp::initWidget()
{
}

void PageSettingsMcp::initItems()
{
    // m_listWidgetMcpServers
    m_listWidgetMcpServers = new QListWidget(this);
    connect(m_listWidgetMcpServers, &QListWidget::itemClicked, this, &PageSettingsMcp::slot_onListWidgetItemClicked);

#ifdef QT_DEBUG
    for (int i = 0; i < 30; ++i)
    {
        QString name = QString("McpServer测试实例%1").arg(i + 1);
        QListWidgetItem *itemMcpServer = new QListWidgetItem(name, m_listWidgetMcpServers);
        itemMcpServer->setData(Qt::UserRole, generateUuid());
    }
#endif
}

void PageSettingsMcp::initLayout()
{
    // splitter
    QSplitter *splitter = new QSplitter(this);
    splitter->setChildrenCollapsible(false);
    splitter->addWidget(m_listWidgetMcpServers);
    splitter->addWidget(new BaseWidget(this));
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 8);
    // vLayout
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addWidget(splitter);
}

void PageSettingsMcp::slot_onListWidgetItemClicked(QListWidgetItem *item)
{
    const QString &mcpServerName = item->data(Qt::DisplayRole).value<QString>();
    const QString &mcpServerUuid = item->data(Qt::UserRole).value<QString>();
    LOG_DEBUG("选中agent: {} - {}", mcpServerName, mcpServerUuid);
    showMcpServerInfo(mcpServerUuid);
}

void PageSettingsMcp::showMcpServerInfo(const QString &uuid)
{
    std::shared_ptr<McpServer> mcpServer = DataManager::getInstance()->getMcpServer(uuid);
    // TODO 展示mcp服务器信息
}

// PageSettingsData
PageSettingsData::PageSettingsData(QWidget *parent)
    : BaseWidget(parent)
{
    initUI();
}

void PageSettingsData::initWidget()
{
}

void PageSettingsData::initItems()
{
    QLabel *label = new QLabel("PageSettingsData", this);
}

void PageSettingsData::initLayout()
{
}

// PageAbout
PageAbout::PageAbout(QWidget *parent)
    : BaseWidget(parent)
{
    initUI();
}

void PageAbout::initWidget()
{
}

void PageAbout::initItems()
{
    QLabel *label = new QLabel("PageAbout", this);
}

void PageAbout::initLayout()
{
}
