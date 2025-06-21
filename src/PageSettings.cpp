#include "PageSettings.h"
#include <QSplitter>
#include "Logger.hpp"
#include <QVBoxLayout>
#include "global.h"
#include <QGridLayout>
#include <QDesktopServices>
#include <QGroupBox>
#include <QFileDialog>

// PageSettings
PageSettings::PageSettings(QWidget *parent)
    : BaseWidget(parent)
{
    initUI();
    // pageSettingsLLM
    addPage("模型服务", new PageSettingsLLM(this));
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
    // 默认选中并展示第一项
    if (m_listWidget->currentItem() == nullptr)
    {
        m_listWidget->setCurrentRow(0);
    }
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

// PageSettingsLLM
PageSettingsLLM::PageSettingsLLM(QWidget *parent)
    : BaseWidget(parent)
{
    initUI();
    connect(DataManager::getInstance(), &DataManager::sig_LLMsLoaded, this, &PageSettingsLLM::slot_onLLMsLoaded);
}

void PageSettingsLLM::initWidget()
{
}

void PageSettingsLLM::initItems()
{
    // m_listWidgetLLMs
    m_listWidgetLLMs = new QListWidget(this);
    connect(m_listWidgetLLMs, &QListWidget::itemClicked, this, &PageSettingsLLM::slot_onListWidgetItemClicked);
    // m_widgetLLMInfo
    m_widgetLLMInfo = new WidgetLLMInfo(this);
}

void PageSettingsLLM::initLayout()
{
    // splitter
    QSplitter *splitter = new QSplitter(this);
    splitter->setChildrenCollapsible(false);
    splitter->addWidget(m_listWidgetLLMs);
    splitter->addWidget(m_widgetLLMInfo);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 8);
    // vLayout
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addWidget(splitter);
}

void PageSettingsLLM::slot_onListWidgetItemClicked(QListWidgetItem *item)
{
    const QString &llmName = item->data(Qt::DisplayRole).value<QString>();
    const QString &llmUuid = item->data(Qt::UserRole).value<QString>();
    LOG_DEBUG("选中llm: {} - {}", llmName, llmUuid);
    showLLMInfo(llmUuid);
}

void PageSettingsLLM::slot_onLLMsLoaded(bool success)
{
    if (!success)
        return;
    QList<std::shared_ptr<LLM>> llms = DataManager::getInstance()->getLLMs();
    if (llms.isEmpty())
        return;
    m_listWidgetLLMs->clear();
    for (const std::shared_ptr<LLM> &llm : llms)
    {
        QListWidgetItem *itemLLM = new QListWidgetItem(llm->modelName, m_listWidgetLLMs);
        itemLLM->setData(Qt::UserRole, QVariant::fromValue<QString>(llm->uuid));
        m_listWidgetLLMs->addItem(itemLLM);
    }
    m_listWidgetLLMs->sortItems();
    // 默认选中并展示第一项
    if (m_listWidgetLLMs->currentItem() == nullptr)
    {
        m_listWidgetLLMs->setCurrentRow(0);
        const std::shared_ptr<LLM> &llm = DataManager::getInstance()->getLLM(m_listWidgetLLMs->currentItem()->data(Qt::UserRole).value<QString>());
        if (!llm)
            return;
        m_widgetLLMInfo->updateData(llm);
    }
}

void PageSettingsLLM::showLLMInfo(const QString &uuid)
{
    const std::shared_ptr<LLM> &llm = DataManager::getInstance()->getLLM(uuid);
    if (!llm)
    {
        LOG_WARN("不存在的llm: {}", uuid);
        return;
    }
    m_widgetLLMInfo->updateData(llm);
}

// WidgetLLMInfo
WidgetLLMInfo::WidgetLLMInfo(QWidget *parent)
{
    initUI();
}

void WidgetLLMInfo::initWidget()
{
}

void WidgetLLMInfo::initItems()
{
    // m_lineEditUuid
    m_lineEditUuid = new QLineEdit(this);
    m_lineEditUuid->setReadOnly(true);
    m_lineEditUuid->setPlaceholderText("UUID");
    // m_lineEditModelID
    m_lineEditModelID = new QLineEdit(this);
    m_lineEditModelID->setPlaceholderText("deepseek-chat");
    // m_lineEditModelName
    m_lineEditModelName = new QLineEdit(this);
    m_lineEditModelName->setPlaceholderText("DeepSeek-V3-0324");
    // m_lineEditApiKey
    m_lineEditApiKey = new QLineEdit(this);
    m_lineEditApiKey->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    m_lineEditApiKey->setPlaceholderText("sk-12345ab123ab12abcd1a123456a1ab12");
    // m_lineEditBaseUrl
    m_lineEditBaseUrl = new QLineEdit(this);
    m_lineEditBaseUrl->setPlaceholderText("https://api.deepseek.com");
    // m_lineEditEndPoint
    m_lineEditEndPoint = new QLineEdit(this);
    m_lineEditEndPoint->setPlaceholderText("/v1/chat/completions");
    // m_pushButtonAdd
    m_pushButtonAdd = new QPushButton("添加", this);
    connect(m_pushButtonAdd, &QPushButton::clicked, this,
            []()
            {
                LOG_DEBUG("添加新模型");
            });
    // m_pushButtonReset
    m_pushButtonReset = new QPushButton("重置", this);
    connect(m_pushButtonReset, &QPushButton::clicked, this,
            [this]()
            {
                const std::shared_ptr<LLM> &llm = DataManager::getInstance()->getLLM(m_lineEditUuid->text());
                if (!llm)
                    return;
                updateData(llm);
                LOG_DEBUG("重载llm: {}", m_lineEditUuid->text());
            });
    // m_pushButtonSave
    m_pushButtonSave = new QPushButton("保存", this);
    connect(m_pushButtonSave, &QPushButton::clicked, this,
            [this]()
            {
                DataManager::getInstance()->updateLLM(getCurrentData());
                LOG_DEBUG("保存llm: {}", m_lineEditUuid->text());
            });
}

void WidgetLLMInfo::initLayout()
{
    // hLayoutButtons
    QHBoxLayout *hLayoutButtons = new QHBoxLayout();
    hLayoutButtons->setContentsMargins(0, 0, 0, 0);
    hLayoutButtons->addWidget(m_pushButtonAdd);
    hLayoutButtons->addStretch();
    hLayoutButtons->addWidget(m_pushButtonReset);
    hLayoutButtons->addWidget(m_pushButtonSave);
    // gLayout
    QGridLayout *gLayout = new QGridLayout();
    gLayout->setContentsMargins(0, 0, 0, 0);
    gLayout->addWidget(new QLabel("UUID", this), 0, 0);
    gLayout->addWidget(m_lineEditUuid, 0, 1);
    gLayout->addWidget(new QLabel("模型ID", this), 1, 0);
    gLayout->addWidget(m_lineEditModelID, 1, 1);
    gLayout->addWidget(new QLabel("模型名称", this), 2, 0);
    gLayout->addWidget(m_lineEditModelName, 2, 1);
    gLayout->addWidget(new QLabel("API密钥", this), 3, 0);
    gLayout->addWidget(m_lineEditApiKey, 3, 1);
    gLayout->addWidget(new QLabel("API地址", this), 4, 0);
    gLayout->addWidget(m_lineEditBaseUrl, 4, 1);
    gLayout->addWidget(new QLabel("接口", this), 5, 0);
    gLayout->addWidget(m_lineEditEndPoint, 5, 1);
    // vLayout
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addLayout(gLayout);
    vLayout->addLayout(hLayoutButtons);
    vLayout->addStretch();
}

void WidgetLLMInfo::updateData(std::shared_ptr<LLM> llm)
{
    if (!llm)
        return;
    m_lineEditUuid->setText(llm->uuid);
    m_lineEditModelID->setText(llm->modelID);
    m_lineEditModelName->setText(llm->modelName);
    m_lineEditApiKey->setText(llm->apiKey);
    m_lineEditBaseUrl->setText(llm->baseUrl);
    m_lineEditEndPoint->setText(llm->endPoint);
}

std::shared_ptr<LLM> WidgetLLMInfo::getCurrentData()
{
    std::shared_ptr<LLM> llm = std::make_shared<LLM>();
    llm->uuid = m_lineEditUuid->text();
    llm->modelID = m_lineEditModelID->text();
    llm->modelName = m_lineEditModelName->text();
    llm->apiKey = m_lineEditApiKey->text();
    llm->baseUrl = m_lineEditBaseUrl->text();
    llm->endPoint = m_lineEditEndPoint->text();
    return llm;
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
    m_listWidgetAgents->clear();
    for (const std::shared_ptr<Agent> &agent : agents)
    {
        QListWidgetItem *itemAgent = new QListWidgetItem(agent->name, m_listWidgetAgents);
        itemAgent->setData(Qt::UserRole, QVariant::fromValue<QString>(agent->uuid));
        m_listWidgetAgents->addItem(itemAgent);
    }
    m_listWidgetAgents->sortItems();
    // 默认选中并展示第一项
    if (m_listWidgetAgents->currentItem() == nullptr)
    {
        m_listWidgetAgents->setCurrentRow(0);
        const std::shared_ptr<Agent> &agent = DataManager::getInstance()->getAgent(m_listWidgetAgents->currentItem()->data(Qt::UserRole).value<QString>());
        if (!agent)
            return;
        m_widgetAgentInfo->updateData(agent);
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
    m_spinBoxChildren->setRange(0, 9999999);
    // m_plainTextEditDescription
    m_plainTextEditDescription = new QPlainTextEdit(this);
    m_plainTextEditDescription->setPlaceholderText("描述");
    // m_lineEditModelUuid
    m_lineEditModelUuid = new QLineEdit(this);
    m_lineEditModelUuid->setPlaceholderText("模型");
    // m_spinBoxContext
    m_spinBoxContext = new QSpinBox(this);
    m_spinBoxContext->setRange(0, 9999999);
    // m_doubleSpinBoxTemperature
    m_doubleSpinBoxTemperature = new QDoubleSpinBox(this);
    m_doubleSpinBoxTemperature->setRange(0, 1);
    // m_doubleSpinBoxTopP
    m_doubleSpinBoxTopP = new QDoubleSpinBox(this);
    m_doubleSpinBoxTopP->setRange(0, 1);
    // m_spinBoxMaxTokens
    m_spinBoxMaxTokens = new QSpinBox(this);
    m_spinBoxMaxTokens->setRange(0, 9999999);
    // m_plainTextEditSystemPrompt
    m_plainTextEditSystemPrompt = new QPlainTextEdit(this);
    m_plainTextEditSystemPrompt->setPlaceholderText("系统提示词");
    // m_listWidgetMcpServers
    m_listWidgetMcpServers = new QListWidget(this);
    // m_pushButtonReset
    m_pushButtonReset = new QPushButton("重置", this);
    connect(m_pushButtonReset, &QPushButton::clicked, this,
            [this]()
            {
                const std::shared_ptr<Agent> &agent = DataManager::getInstance()->getAgent(m_lineEditUuid->text());
                if (!agent)
                    return;
                updateData(agent);
                LOG_DEBUG("重载agent: {}", m_lineEditUuid->text());
            });
    // m_pushButtonSave
    m_pushButtonSave = new QPushButton("保存", this);
    connect(m_pushButtonSave, &QPushButton::clicked, this,
            [this]()
            {
                DataManager::getInstance()->updateAgent(getCurrentData());
                LOG_DEBUG("保存agent: {}", m_lineEditUuid->text());
            });
}

void WidgetAgentInfo::initLayout()
{
    // hLayoutButtons
    QHBoxLayout *hLayoutButtons = new QHBoxLayout();
    hLayoutButtons->setContentsMargins(0, 0, 0, 0);
    hLayoutButtons->addStretch();
    hLayoutButtons->addWidget(m_pushButtonReset);
    hLayoutButtons->addWidget(m_pushButtonSave);
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
    gLayout->addWidget(new QLabel("模型", this), 4, 0);
    gLayout->addWidget(m_lineEditModelUuid, 4, 1);
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
    gLayout->addLayout(hLayoutButtons, 11, 1);
}

void WidgetAgentInfo::updateData(std::shared_ptr<Agent> agent)
{
    if (!agent)
        return;
    m_lineEditUuid->setText(agent->uuid);
    m_lineEditName->setText(agent->name);
    m_spinBoxChildren->setValue(agent->children);
    m_plainTextEditDescription->setPlainText(agent->description);
    m_lineEditModelUuid->setText(agent->modelUuid);
    m_spinBoxContext->setValue(agent->context);
    m_doubleSpinBoxTemperature->setValue(agent->temperature);
    m_doubleSpinBoxTopP->setValue(agent->topP);
    m_spinBoxMaxTokens->setValue(agent->maxTokens);
    m_plainTextEditSystemPrompt->setPlainText(agent->systemPrompt);
    m_listWidgetMcpServers->clear();
    for (const QString &uuid : agent->mcpServers)
    {
        const std::shared_ptr<McpServer> &mcpServer = DataManager::getInstance()->getMcpServer(uuid);
        if (!mcpServer)
        {
            LOG_WARN("不存在的mcp服务器: {}", uuid);
            continue;
        }
        QListWidgetItem *itemMcpServer = new QListWidgetItem(mcpServer->name, m_listWidgetMcpServers);
        itemMcpServer->setData(Qt::UserRole, QVariant::fromValue<QString>(mcpServer->uuid));
        m_listWidgetMcpServers->addItem(itemMcpServer);
    }
}

std::shared_ptr<Agent> WidgetAgentInfo::getCurrentData()
{
    std::shared_ptr<Agent> agent = std::make_shared<Agent>();
    agent->uuid = m_lineEditUuid->text();
    agent->name = m_lineEditName->text();
    agent->children = m_spinBoxChildren->value();
    agent->description = m_plainTextEditDescription->toPlainText();
    agent->modelUuid = m_lineEditModelUuid->text();
    agent->context = m_spinBoxContext->value();
    agent->temperature = m_doubleSpinBoxTemperature->value();
    agent->topP = m_doubleSpinBoxTopP->value();
    agent->maxTokens = m_spinBoxMaxTokens->value();
    agent->systemPrompt = m_plainTextEditSystemPrompt->toPlainText();
    agent->mcpServers.clear();
    for (int i = 0; i < m_listWidgetMcpServers->count(); ++i)
    {
        QListWidgetItem *item = m_listWidgetMcpServers->item(i);
        QString uuid = item->data(Qt::UserRole).toString();
        if (!uuid.isEmpty())
            agent->mcpServers.append(uuid);
    }
    return agent;
}

// PageSettingsMcp
PageSettingsMcp::PageSettingsMcp(QWidget *parent)
    : BaseWidget(parent)
{
    initUI();
    connect(DataManager::getInstance(), &DataManager::sig_mcpServersLoaded, this, &PageSettingsMcp::slot_onMcpServersLoaded);
}

void PageSettingsMcp::initWidget()
{
}

void PageSettingsMcp::initItems()
{
    // m_listWidgetMcpServers
    m_listWidgetMcpServers = new QListWidget(this);
    connect(m_listWidgetMcpServers, &QListWidget::itemClicked, this, &PageSettingsMcp::slot_onListWidgetItemClicked);
    // m_widgetMcpServerInfo
    m_widgetMcpServerInfo = new WidgetMcpServerInfo(this);
}

void PageSettingsMcp::initLayout()
{
    // splitter
    QSplitter *splitter = new QSplitter(this);
    splitter->setChildrenCollapsible(false);
    splitter->addWidget(m_listWidgetMcpServers);
    splitter->addWidget(m_widgetMcpServerInfo);
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
    LOG_DEBUG("选中mcp服务器: {} - {}", mcpServerName, mcpServerUuid);
    showMcpServerInfo(mcpServerUuid);
}

void PageSettingsMcp::slot_onMcpServersLoaded(bool success)
{
    if (!success)
        return;
    QList<std::shared_ptr<McpServer>> mcpServers = DataManager::getInstance()->getMcpServers();
    if (mcpServers.isEmpty())
        return;
    m_listWidgetMcpServers->clear();
    for (const std::shared_ptr<McpServer> &mcpServer : mcpServers)
    {
        QListWidgetItem *itemAgent = new QListWidgetItem(mcpServer->name, m_listWidgetMcpServers);
        itemAgent->setData(Qt::UserRole, QVariant::fromValue<QString>(mcpServer->uuid));
        m_listWidgetMcpServers->addItem(itemAgent);
    }
    m_listWidgetMcpServers->sortItems();
    // 默认选中并展示第一项
    if (m_listWidgetMcpServers->currentItem() == nullptr)
    {
        m_listWidgetMcpServers->setCurrentRow(0);
        m_widgetMcpServerInfo->updateData(DataManager::getInstance()->getMcpServer(m_listWidgetMcpServers->currentItem()->data(Qt::UserRole).value<QString>()));
    }
}

void PageSettingsMcp::showMcpServerInfo(const QString &uuid)
{
    const std::shared_ptr<McpServer> &mcpServer = DataManager::getInstance()->getMcpServer(uuid);
    if (!mcpServer)
    {
        LOG_WARN("不存在的mcpServer: {}", uuid);
        return;
    }
    // 展示mcp服务器信息
    m_widgetMcpServerInfo->updateData(mcpServer);
}

// WidgetMcpServerInfo
WidgetMcpServerInfo::WidgetMcpServerInfo(QWidget *parent)
{
    initUI();
}

void WidgetMcpServerInfo::initWidget()
{
}

void WidgetMcpServerInfo::initItems()
{
    // m_lineEditUuid
    m_lineEditUuid = new QLineEdit(this);
    m_lineEditUuid->setReadOnly(true);
    m_lineEditUuid->setPlaceholderText("UUID");
    // m_lineEditName
    m_lineEditName = new QLineEdit(this);
    m_lineEditName->setPlaceholderText("名称");
    // m_plainTextEditDescription
    m_plainTextEditDescription = new QPlainTextEdit(this);
    m_plainTextEditDescription->setPlaceholderText("描述");
    // m_comboBoxType
    m_comboBoxType = new QComboBox(this);
    m_comboBoxType->addItem("标准输入/输出(stdio)");
    m_comboBoxType->addItem("服务器发送事件(sse)");
    m_comboBoxType->addItem("可流式传输的HTTP(streamableHttp)");
    // connect(m_comboBoxType, &QComboBox::currentIndexChanged, this, &WidgetMcpServerInfo::slot_onComboBoxCurrentIndexChanged);
    connect(m_comboBoxType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &WidgetMcpServerInfo::slot_onComboBoxCurrentIndexChanged);
    // m_spinBoxTimeout
    m_spinBoxTimeout = new QSpinBox(this);
    m_spinBoxTimeout->setRange(0, 9999999);
    // m_labelCommand
    m_labelCommand = new QLabel("命令", this);
    // m_lineEditCommand
    m_lineEditCommand = new QLineEdit(this);
    m_lineEditCommand->setPlaceholderText("uvx or npm");
    // m_labelArgs
    m_labelArgs = new QLabel("参数", this);
    // m_plainTextEditArgs
    m_plainTextEditArgs = new QPlainTextEdit(this);
    m_plainTextEditArgs->setPlaceholderText("arg1\narg2");
    // m_labelEnvVars
    m_labelEnvVars = new QLabel("环境变量", this);
    // m_plainTextEditEnvVars
    m_plainTextEditEnvVars = new QPlainTextEdit(this);
    m_plainTextEditEnvVars->setPlaceholderText("KEY1=value1\nKEY2-value2");
    // m_labelUrl
    m_labelUrl = new QLabel("URL", this);
    // m_lineEditUrl
    m_lineEditUrl = new QLineEdit(this);
    m_lineEditUrl->setPlaceholderText("http://localhost:3000/sse");
    // m_labelRequestHeaders
    m_labelRequestHeaders = new QLabel("请求头", this);
    // m_plainTextEditRequestHeaders
    m_plainTextEditRequestHeaders = new QPlainTextEdit(this);
    m_plainTextEditRequestHeaders->setPlaceholderText("Content-Type=application/json\nAuthorization=Bearer token");
    // m_pushButtonReset
    m_pushButtonReset = new QPushButton("重置", this);
    connect(m_pushButtonReset, &QPushButton::clicked, this,
            [this]()
            {
                updateData(DataManager::getInstance()->getMcpServer(m_lineEditUuid->text()));
                LOG_DEBUG("重载mcp服务器: {}", m_lineEditUuid->text());
            });
    // m_pushButtonSave
    m_pushButtonSave = new QPushButton("保存", this);
    connect(m_pushButtonSave, &QPushButton::clicked, this,
            [this]()
            {
                DataManager::getInstance()->updateMcpServer(getCurrentData());
                LOG_DEBUG("保存mcp服务器: {}", m_lineEditUuid->text());
            });
}

void WidgetMcpServerInfo::initLayout()
{
    // hLayoutButtons
    QHBoxLayout *hLayoutButtons = new QHBoxLayout();
    hLayoutButtons->setContentsMargins(0, 0, 0, 0);
    hLayoutButtons->addStretch();
    hLayoutButtons->addWidget(m_pushButtonReset);
    hLayoutButtons->addWidget(m_pushButtonSave);
    // gLayout
    QGridLayout *gLayout = new QGridLayout(this);
    gLayout->addWidget(new QLabel("UUID", this), 0, 0);
    gLayout->addWidget(m_lineEditUuid, 0, 1);
    gLayout->addWidget(new QLabel("名称", this), 1, 0);
    gLayout->addWidget(m_lineEditName, 1, 1);
    gLayout->addWidget(new QLabel("描述", this), 2, 0);
    gLayout->addWidget(m_plainTextEditDescription, 2, 1);
    gLayout->addWidget(new QLabel("类型", this), 3, 0);
    gLayout->addWidget(m_comboBoxType, 3, 1);
    gLayout->addWidget(new QLabel("超时", this), 4, 0);
    gLayout->addWidget(m_spinBoxTimeout, 4, 1);
    gLayout->addWidget(m_labelCommand, 5, 0);
    gLayout->addWidget(m_lineEditCommand, 5, 1);
    gLayout->addWidget(m_labelArgs, 6, 0);
    gLayout->addWidget(m_plainTextEditArgs, 6, 1);
    gLayout->addWidget(m_labelEnvVars, 7, 0);
    gLayout->addWidget(m_plainTextEditEnvVars, 7, 1);
    gLayout->addWidget(m_labelUrl, 8, 0);
    gLayout->addWidget(m_lineEditUrl, 8, 1);
    gLayout->addWidget(m_labelRequestHeaders, 9, 0);
    gLayout->addWidget(m_plainTextEditRequestHeaders, 9, 1);
    gLayout->addLayout(hLayoutButtons, 11, 1);
}

void WidgetMcpServerInfo::updateData(std::shared_ptr<McpServer> mcpServer)
{
    if (!mcpServer)
        return;
    m_lineEditUuid->setText(mcpServer->uuid);
    m_lineEditName->setText(mcpServer->name);
    m_plainTextEditDescription->setPlainText(mcpServer->description);
    m_comboBoxType->setCurrentIndex(mcpServer->type);
    m_spinBoxTimeout->setValue(mcpServer->timeout);
    m_lineEditCommand->setText(mcpServer->command);
    QString strAgrs;
    for (const QString &arg : mcpServer->args)
    {
        strAgrs.append(arg).append("\n");
    }
    m_plainTextEditArgs->setPlainText(strAgrs);
    QString strEnvVars;
    for (auto it = mcpServer->envVars.constBegin(); it != mcpServer->envVars.constEnd(); ++it)
    {
        strEnvVars.append(it.key()).append("=").append(it.value()).append("\n");
    }
    m_plainTextEditEnvVars->setPlainText(strEnvVars);
    m_lineEditUrl->setText(mcpServer->url);
    m_plainTextEditRequestHeaders->setPlainText(mcpServer->requestHeaders);
}

std::shared_ptr<McpServer> WidgetMcpServerInfo::getCurrentData()
{
    std::shared_ptr<McpServer> mcpServer = std::make_shared<McpServer>();
    mcpServer->uuid = m_lineEditUuid->text();
    mcpServer->name = m_lineEditName->text();
    mcpServer->description = m_plainTextEditDescription->toPlainText();
    mcpServer->type = static_cast<McpServer::Type>(m_comboBoxType->currentIndex());
    mcpServer->timeout = m_spinBoxTimeout->value();
    mcpServer->command = m_lineEditCommand->text();

    // 解析参数
    mcpServer->args.clear();
    QStringList argsList = m_plainTextEditArgs->toPlainText().split('\n', Qt::SkipEmptyParts);
    for (const QString &arg : argsList)
    {
        mcpServer->args.append(arg.trimmed());
    }

    // 解析环境变量
    mcpServer->envVars.clear();
    QStringList envList = m_plainTextEditEnvVars->toPlainText().split('\n', Qt::SkipEmptyParts);
    for (const QString &env : envList)
    {
        QString trimmedEnv = env.trimmed();
        if (trimmedEnv.isEmpty())
            continue;
        int idx = trimmedEnv.indexOf('=');
        if (idx > 0)
        {
            QString key = trimmedEnv.left(idx).trimmed();
            QString value = trimmedEnv.mid(idx + 1).trimmed();
            if (!key.isEmpty())
                mcpServer->envVars.insert(key, value);
        }
    }

    mcpServer->url = m_lineEditUrl->text();
    mcpServer->requestHeaders = m_plainTextEditRequestHeaders->toPlainText();

    return mcpServer;
}

void WidgetMcpServerInfo::slot_onComboBoxCurrentIndexChanged(int index)
{
    if (index == McpServer::Type::stdio)
    {
        m_labelCommand->show();
        m_lineEditCommand->show();
        m_labelArgs->show();
        m_plainTextEditArgs->show();
        m_labelEnvVars->show();
        m_plainTextEditEnvVars->show();
        m_labelUrl->hide();
        m_lineEditUrl->hide();
        m_labelRequestHeaders->hide();
        m_plainTextEditRequestHeaders->hide();
        return;
    }
    if (index == McpServer::Type::sse || McpServer::Type::streambleHttp)
    {
        m_labelCommand->hide();
        m_lineEditCommand->hide();
        m_labelArgs->hide();
        m_plainTextEditArgs->hide();
        m_labelEnvVars->hide();
        m_plainTextEditEnvVars->hide();
        m_labelUrl->show();
        m_lineEditUrl->show();
        m_labelRequestHeaders->show();
        m_plainTextEditRequestHeaders->show();
        return;
    }
    LOG_WARN("不存在的mcp服务器类型: {}", m_comboBoxType->currentText());
}

// PageSettingsData
PageSettingsData::PageSettingsData(QWidget *parent)
    : BaseWidget(parent)
{
    initUI();
    connect(DataManager::getInstance(), &DataManager::sig_filePathChangedAgents, this, &PageSettingsData::slot_onFilePathChangedAgents);
    connect(DataManager::getInstance(), &DataManager::sig_filePathChangedMcpServers, this, &PageSettingsData::slot_onFilePathChangedMcpServers);
}

void PageSettingsData::initWidget()
{
}

void PageSettingsData::initItems()
{
    // m_lineEditFilePathAgents
    m_lineEditFilePathAgents = new QLineEdit(this);
    m_lineEditFilePathAgents->setText(QFileInfo(DataManager::getInstance()->getFilePathAgents()).absoluteFilePath());
    // m_pushButtonSelectFileAgents
    m_pushButtonSelectFileAgents = new QPushButton("选择", this);
    connect(m_pushButtonSelectFileAgents, &QPushButton::clicked, this,
            [this]()
            {
                QString fileName = QFileDialog::getOpenFileName(this, "选择 Agents 文件", QString(), "JSON Files (*.json);;All Files (*)");
                if (!fileName.isEmpty())
                {
                    DataManager::getInstance()->setFilePathAgents(fileName);
                    LOG_DEBUG("设置agents文件路径为: {}", fileName);
                }
            });
    // m_lineEditFilePathMcpServers
    m_lineEditFilePathMcpServers = new QLineEdit(this);
    m_lineEditFilePathMcpServers->setText(QFileInfo(DataManager::getInstance()->getFilePathMcpServers()).absoluteFilePath());
    // m_pushButtonSelectFileMcpServers
    m_pushButtonSelectFileMcpServers = new QPushButton("选择", this);
    connect(m_pushButtonSelectFileMcpServers, &QPushButton::clicked, this,
            [this]()
            {
                QString fileName = QFileDialog::getOpenFileName(this, "选择 McpServers 文件", QString(), "JSON Files (*.json);;All Files (*)");
                if (!fileName.isEmpty())
                {
                    DataManager::getInstance()->setFilePathMcpServers(fileName);
                    LOG_DEBUG("设置mcp服务器文件路径为: {}", fileName);
                }
            });
}

void PageSettingsData::initLayout()
{
    // gLayoutStorage
    QGridLayout *gLayoutStorage = new QGridLayout();
    gLayoutStorage->addWidget(new QLabel("Agents", this), 0, 0);
    gLayoutStorage->addWidget(m_lineEditFilePathAgents, 0, 1);
    gLayoutStorage->addWidget(m_pushButtonSelectFileAgents, 0, 2);
    gLayoutStorage->addWidget(new QLabel("McpServers", this), 1, 0);
    gLayoutStorage->addWidget(m_lineEditFilePathMcpServers, 1, 1);
    gLayoutStorage->addWidget(m_pushButtonSelectFileMcpServers, 1, 2);
    // groupBoxStorage
    QGroupBox *groupBoxStorage = new QGroupBox("存储设置", this);
    groupBoxStorage->setLayout(gLayoutStorage);
    // vLayout
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addWidget(groupBoxStorage);
    vLayout->addStretch();
}

void PageSettingsData::slot_onFilePathChangedAgents(const QString &filePath)
{
    if (filePath.isEmpty())
        return;
    m_lineEditFilePathAgents->setText(QFileInfo(filePath).absoluteFilePath());
}

void PageSettingsData::slot_onFilePathChangedMcpServers(const QString &filePath)
{
    if (filePath.isEmpty())
        return;
    m_lineEditFilePathMcpServers->setText(QFileInfo(filePath).absoluteFilePath());
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
    // m_pushButtonGitHub
    m_pushButtonGitHub = new QPushButton("GitHub", this);
    connect(m_pushButtonGitHub, &QPushButton::clicked, this,
            []()
            {
                QDesktopServices::openUrl(QUrl("https://github.com/zcy946/XLC-Assistant"));
                LOG_DEBUG("跳转到GitHub");
            });
    // m_pushButtonBlog
    m_pushButtonBlog = new QPushButton("Blog", this);
    connect(m_pushButtonBlog, &QPushButton::clicked, this,
            []()
            {
                QDesktopServices::openUrl(QUrl("https://1610161.xyz"));
                LOG_DEBUG("跳转到Blog");
            });
}

void PageAbout::initLayout()
{
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addWidget(new QLabel("XLCMCPClient\n——by: xialichen", this));
    vLayout->addWidget(m_pushButtonGitHub, 0, Qt::AlignRight);
    vLayout->addWidget(m_pushButtonBlog, 0, Qt::AlignRight);
    vLayout->addStretch();
}
