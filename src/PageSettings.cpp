#include "PageSettings.h"
#include <QSplitter>
#include "Logger.hpp"
#include <QVBoxLayout>
#include "global.h"
#include <QGridLayout>
#include <QDesktopServices>
#include <QGroupBox>
#include <QFileDialog>
#include "EventBus.h"
#include <QJsonObject>
#include <QMessageBox>
#include "ColorRepository.h"

// PageSettings
PageSettings::PageSettings(QWidget *parent)
    : BaseWidget(parent)
{
    initUI();
    // pageSettingsAgent
    addPage("智能体", new PageSettingsAgent(this));
    // pageSettingsLLM
    addPage("模型服务", new PageSettingsLLM(this));
    // pageSettingsMcp
    addPage("MCP 服务器", new PageSettingsMcp(this));
    // pageSettingData
    addPage("存储设置", new PageSettingsStorage(this));
    // PageSettingsDisplay
    addPage("显示设置", new PageSettingsDisplay(this));
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
                    XLC_LOG_WARN("Non-existent setting item ({})", currentText);
                    return;
                }
                m_stackedWidget->setCurrentWidget(it.value());
                XLC_LOG_TRACE("Setting item selected (item={})", currentText);
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
    : BaseSettingsPage(
          "Agent",
          []()
          { return DataManager::getInstance()->getAgents(); },
          [](const QString &uuid)
          { return DataManager::getInstance()->getAgent(uuid); },
          [](std::shared_ptr<Agent> agent)
          { DataManager::getInstance()->addAgent(agent); },
          [](std::shared_ptr<Agent> agent)
          { DataManager::getInstance()->updateAgent(agent); },
          [](const QString &uuid)
          { DataManager::getInstance()->removeAgent(uuid); },
          [](const std::shared_ptr<Agent> &agent)
          { return agent->name; },
          EventBus::States::AGENT_UPDATED,
          parent)
{
    connectDataManagerDataLoadedSignals();
}

void PageSettingsAgent::connectDataManagerDataLoadedSignals()
{
    connect(DataManager::getInstance(), &DataManager::sig_agentsLoaded, this, &PageSettingsAgent::slot_onDataLoaded);
    connect(DataManager::getInstance(), &DataManager::sig_mcpServersLoaded, this, &PageSettingsAgent::slot_onDataLoaded);
    connect(DataManager::getInstance(), &DataManager::sig_conversationsLoaded, this, &PageSettingsAgent::slot_onDataLoaded);
}

// WidgetAgentInfo
WidgetAgentInfo::WidgetAgentInfo(QWidget *parent)
{
    initUI();
    connect(DataManager::getInstance(), &DataManager::sig_LLMsLoaded, this, &WidgetAgentInfo::slot_onLLMsLoaded);
    connect(EventBus::getInstance().get(), &EventBus::sig_stateChanged, this, &WidgetAgentInfo::slot_handleStateChanged);
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
    // m_plainTextEditDescription
    m_plainTextEditDescription = new QPlainTextEdit(this);
    m_plainTextEditDescription->setPlaceholderText("描述");
    // m_comboBoxLLM
    m_comboBoxLLM = new QComboBox(this);
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
    // 添加右键菜单
    // 右键点击时会发出 customContextMenuRequested 信号
    m_listWidgetMcpServers->setContextMenuPolicy(Qt::CustomContextMenu);
    // m_contextMenuMcpServers
    m_contextMenuMcpServers = new QMenu(m_listWidgetMcpServers);
    QAction *actionEditMcpServer = new QAction("编辑", m_listWidgetMcpServers);
    QAction *actionDeleteMcpServer = new QAction("删除", m_listWidgetMcpServers);
    m_contextMenuMcpServers->addAction(actionEditMcpServer);
    m_contextMenuMcpServers->addAction(actionDeleteMcpServer);
    connect(m_listWidgetMcpServers, &QListWidget::customContextMenuRequested, this,
            [this, actionDeleteMcpServer](const QPoint &pos)
            {
                QListWidgetItem *rightClickedItem = m_listWidgetMcpServers->itemAt(pos);
                if (rightClickedItem)
                {
                    actionDeleteMcpServer->setEnabled(true);
                }
                else
                {
                    actionDeleteMcpServer->setEnabled(false);
                }
                m_contextMenuMcpServers->exec(m_listWidgetMcpServers->mapToGlobal(pos));
            });
    connect(actionEditMcpServer, &QAction::triggered, this,
            [this]()
            {
                XLC_LOG_DEBUG("Attempting to add (mount) mcp server (agentUuid={})", m_lineEditUuid->text());
                // 获取所有已挂载的mcp服务器uuid
                std::shared_ptr<Agent> agent = DataManager::getInstance()->getAgent(m_lineEditUuid->text());
                if (!agent)
                {
                    XLC_LOG_WARN("Failed to display the list of available MCP servers (agentUuid={}): agent not found", m_lineEditUuid->text());
                    ToastManager::showMessage(Toast::Type::Error, QString("获取可用MCP服务器列表失败 (agentUuid=%1): agent not found").arg(m_lineEditUuid->text()));
                    return;
                }
                std::shared_ptr<QSet<QString>> mountedMCPServerUuidsPtr = std::make_shared<QSet<QString>>(agent->mcpServers);
                DialogMountMcpServer *dialog = new DialogMountMcpServer(mountedMCPServerUuidsPtr, this);
                connect(dialog, &DialogMountMcpServer::finished, this,
                        [this, mountedMCPServerUuidsPtr](int result)
                        {
                            if (result == QDialog::Accepted)
                            {
                                m_listWidgetMcpServers->clear();
                                for (QString uuid : *mountedMCPServerUuidsPtr)
                                {
                                    const std::shared_ptr<McpServer> mcpServer = DataManager::getInstance()->getMcpServer(uuid);
                                    if (!mcpServer || (!mcpServer->isActive && !mountedMCPServerUuidsPtr->contains(mcpServer->uuid)))
                                        continue;
                                    QListWidgetItem *item = new QListWidgetItem(mcpServer->isActive ? mcpServer->name : mcpServer->name + " [未启用]", m_listWidgetMcpServers);
                                    item->setData(Qt::UserRole, QVariant::fromValue<QString>(mcpServer->uuid));
                                    m_listWidgetMcpServers->addItem(item);
                                }
                                XLC_LOG_DEBUG("Updated MCP server list");
                            }
                        });
                dialog->exec();
            });
    connect(actionDeleteMcpServer, &QAction::triggered, this,
            [this]()
            {
                // 删除当前选中项
                QListWidgetItem *selectedItem = m_listWidgetMcpServers->currentItem();
                if (selectedItem)
                {
                    XLC_LOG_DEBUG("Deleting mounted mcp server (agentUuid={}, serverUuid={}, serverName={})", m_lineEditUuid->text(), selectedItem->data(Qt::UserRole).toString(), selectedItem->text());
                    int row = m_listWidgetMcpServers->row(selectedItem);
                    delete m_listWidgetMcpServers->takeItem(row);
                }
            });

    // m_listWidgetConversations
    m_listWidgetConversations = new QListWidget(this);
    m_listWidgetConversations->setContextMenuPolicy(Qt::CustomContextMenu);
    // m_contextMenuConversations
    m_contextMenuConversations = new QMenu(m_listWidgetConversations);
    QAction *actionViewConversation = new QAction("查看", m_listWidgetConversations);
    QAction *actionDeleteConversation = new QAction("删除", m_listWidgetConversations);
    m_contextMenuConversations->addAction(actionViewConversation);
    m_contextMenuConversations->addAction(actionDeleteConversation);
    connect(m_listWidgetConversations, &QListWidget::customContextMenuRequested, this,
            [this, actionViewConversation, actionDeleteConversation](const QPoint &pos)
            {
                QListWidgetItem *rightClickedItem = m_listWidgetConversations->itemAt(pos);
                if (rightClickedItem)
                {
                    actionViewConversation->setEnabled(true);
                    actionDeleteConversation->setEnabled(true);
                }
                else
                {
                    actionViewConversation->setEnabled(false);
                    actionDeleteConversation->setEnabled(false);
                }
                m_contextMenuConversations->exec(m_listWidgetConversations->mapToGlobal(pos));
            });
    connect(actionViewConversation, &QAction::triggered, this,
            [this]()
            {
                QListWidgetItem *selectedItem = m_listWidgetConversations->currentItem();
                if (selectedItem)
                {
                    XLC_LOG_TRACE("Attempting to view conversation (agentUuid={}, conversationId={}, title={})", m_lineEditUuid->text(), selectedItem->data(Qt::UserRole).toString(), selectedItem->text());
                    // 跳转至对话
                    QJsonObject objPageInfo;
                    objPageInfo["id"] = static_cast<int>(EventBus::Pages::CONVERSATION);
                    objPageInfo["agentUuid"] = m_lineEditUuid->text();
                    objPageInfo["conversationUuid"] = selectedItem->data(Qt::UserRole).toString();
                    EventBus::getInstance()->publish(EventBus::EventType::PageSwitched, QVariant(objPageInfo));
                }
            });
    connect(actionDeleteConversation, &QAction::triggered, this,
            [this]()
            {
                // 删除当前选中项
                QListWidgetItem *selectedItem = m_listWidgetConversations->currentItem();
                if (selectedItem)
                {
                    XLC_LOG_DEBUG("Deleting conversation (agentUuid={}, conversationUuid={}, conversationName={})", m_lineEditUuid->text(), selectedItem->data(Qt::UserRole).toString(), selectedItem->text());
                    int row = m_listWidgetConversations->row(selectedItem);
                    delete m_listWidgetConversations->takeItem(row);
                }
            });
}

void WidgetAgentInfo::initLayout()
{
    // gLayout
    QGridLayout *gLayout = new QGridLayout(this);
    gLayout->setContentsMargins(0, 0, 0, 0);
    gLayout->addWidget(new QLabel("UUID", this), 0, 0);
    gLayout->addWidget(m_lineEditUuid, 0, 1);
    gLayout->addWidget(new QLabel("名称", this), 1, 0);
    gLayout->addWidget(m_lineEditName, 1, 1);
    gLayout->addWidget(new QLabel("描述", this), 2, 0);
    gLayout->addWidget(m_plainTextEditDescription, 2, 1);
    gLayout->addWidget(new QLabel("模型", this), 3, 0);
    gLayout->addWidget(m_comboBoxLLM, 3, 1);
    gLayout->addWidget(new QLabel("上下文", this), 4, 0);
    gLayout->addWidget(m_spinBoxContext, 4, 1);
    gLayout->addWidget(new QLabel("模型温度", this), 5, 0);
    gLayout->addWidget(m_doubleSpinBoxTemperature, 5, 1);
    gLayout->addWidget(new QLabel("Top-p", this), 6, 0);
    gLayout->addWidget(m_doubleSpinBoxTopP, 6, 1);
    gLayout->addWidget(new QLabel("最大Token数", this), 7, 0);
    gLayout->addWidget(m_spinBoxMaxTokens, 7, 1);
    gLayout->addWidget(new QLabel("系统提示词", this), 8, 0);
    gLayout->addWidget(m_plainTextEditSystemPrompt, 8, 1);
    gLayout->addWidget(new QLabel("MCP服务器", this), 9, 0);
    gLayout->addWidget(m_listWidgetMcpServers, 9, 1);
    gLayout->addWidget(new QLabel("对话列表", this), 10, 0);
    gLayout->addWidget(m_listWidgetConversations, 10, 1);
}

void WidgetAgentInfo::updateFormData(std::shared_ptr<Agent> agent)
{
    if (!agent)
        return;
    m_lineEditUuid->setText(agent->uuid);
    m_lineEditName->setText(agent->name);
    m_plainTextEditDescription->setPlainText(agent->description);
    updateLLMList();
    std::shared_ptr<LLM> llm = DataManager::getInstance()->getLLM(agent->llmUUid);
    if (!llm)
    {
        XLC_LOG_WARN("Update form data failed (agentUuid={}, LLMUuid={}): LLM not found", agent->uuid, agent->llmUUid);
        ToastManager::showMessage(Toast::Type::Error, QString("更新Agent失败 (agentUuid=%1, LLMUuid=%2): agent not found").arg(agent->uuid).arg(agent->llmUUid));
    }
    else
    {
        m_comboBoxLLM->setCurrentText(llm->modelName);
    }
    m_spinBoxContext->setValue(agent->context);
    m_doubleSpinBoxTemperature->setValue(agent->temperature);
    m_doubleSpinBoxTopP->setValue(agent->topP);
    m_spinBoxMaxTokens->setValue(agent->maxTokens);
    m_plainTextEditSystemPrompt->setPlainText(agent->systemPrompt);

    // 更新MCP服务器列表
    m_listWidgetMcpServers->clear();
    for (const QString &uuid : agent->mcpServers)
    {
        std::shared_ptr<McpServer> mcpServer = DataManager::getInstance()->getMcpServer(uuid);
        if (!mcpServer)
        {
            XLC_LOG_WARN("Update form data failed (agentUuid={}): MCP server not found", uuid);
            continue;
        }
        QListWidgetItem *itemMcpServer = new QListWidgetItem(mcpServer->isActive ? mcpServer->name : mcpServer->name + " [未启用]", m_listWidgetMcpServers);
        itemMcpServer->setData(Qt::UserRole, QVariant::fromValue<QString>(mcpServer->uuid));
        m_listWidgetMcpServers->addItem(itemMcpServer);
    }
    m_listWidgetMcpServers->sortItems();

    // 更新对话列表
    m_listWidgetConversations->clear();
    for (const QString &uuid : agent->conversations)
    {
        std::shared_ptr<Conversation> conversation = DataManager::getInstance()->getConversation(uuid);
        if (!conversation)
        {
            // XLC_LOG_WARN("Update form data (agentUuid={}): Conversation is loading or not found", uuid); // 当MCPServer和Agent loaded的时候会分别触发一次，无需在意
            continue;
        }
        QListWidgetItem *itemConversation = new QListWidgetItem(conversation->summary, m_listWidgetConversations);
        itemConversation->setData(Qt::UserRole, QVariant::fromValue<QString>(conversation->uuid));
        m_listWidgetConversations->addItem(itemConversation);
    }
    m_listWidgetConversations->sortItems();
}

void WidgetAgentInfo::clearFormData()
{
    m_lineEditUuid->setText("");
    m_lineEditName->setText("");
    m_plainTextEditDescription->setPlainText("");
    updateLLMList();
    m_comboBoxLLM->setCurrentIndex(0);
    m_spinBoxContext->setValue(0);
    m_doubleSpinBoxTemperature->setValue(0);
    m_doubleSpinBoxTopP->setValue(0);
    m_spinBoxMaxTokens->setValue(0);
    m_plainTextEditSystemPrompt->setPlainText("");
    m_listWidgetMcpServers->clear();
    m_listWidgetConversations->clear();
}

std::shared_ptr<Agent> WidgetAgentInfo::getCurrentData()
{
    std::shared_ptr<Agent> agent = std::make_shared<Agent>();
    agent->uuid = m_lineEditUuid->text();
    agent->name = m_lineEditName->text();
    agent->description = m_plainTextEditDescription->toPlainText();
    agent->llmUUid = m_comboBoxLLM->currentData().toString();
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
            agent->mcpServers.insert(uuid);
    }
    agent->conversations.clear();
    for (int i = 0; i < m_listWidgetConversations->count(); ++i)
    {
        QListWidgetItem *item = m_listWidgetConversations->item(i);
        QString uuid = item->data(Qt::UserRole).toString();
        if (!uuid.isEmpty())
            agent->conversations.insert(uuid);
    }
    return agent;
}

const QString WidgetAgentInfo::getUuid()
{
    return m_lineEditUuid->text();
}

void WidgetAgentInfo::populateBasicInfo()
{
    m_lineEditUuid->setText(generateUuid());
    slot_onLLMsLoaded(true);
}

void WidgetAgentInfo::updateLLMList()
{
    // 更新llm列表
    m_comboBoxLLM->clear();
    QList<std::shared_ptr<LLM>> llms = DataManager::getInstance()->getLLMs();
    if (llms.count() <= 0)
        return;
    std::sort(llms.begin(), llms.end(),
              [](const std::shared_ptr<LLM> &llm_a, const std::shared_ptr<LLM> &llm_b)
              {
                  return llm_a->modelName < llm_b->modelName;
              });
    for (const std::shared_ptr<LLM> &llm : llms)
    {
        m_comboBoxLLM->addItem(llm->modelName, llm->uuid);
    }

    // 选中正确的llm
    if (!m_lineEditUuid->text().isEmpty() && !m_lineEditName->text().isEmpty())
    {
        std::shared_ptr<Agent> agent = DataManager::getInstance()->getAgent(m_lineEditUuid->text());
        if (!agent)
        {
            XLC_LOG_WARN("Agent not found (uuid={})", m_lineEditUuid->text());
            return;
        }
        std::shared_ptr<LLM> llm = DataManager::getInstance()->getLLM(agent->llmUUid);
        if (!llm)
        {
            XLC_LOG_WARN("LLM not found (llmUuid={})", agent->llmUUid);
            return;
        }
        m_comboBoxLLM->setCurrentText(llm->modelName);
    }
}

void WidgetAgentInfo::updateMCPServerList(const std::shared_ptr<Agent> &agent)
{
    m_listWidgetMcpServers->clear();
    std::shared_ptr<Agent> currentAgent = agent;
    if (!currentAgent)
    {
        currentAgent = DataManager::getInstance()->getAgent(m_lineEditUuid->text());
        if (!currentAgent)
        {
            XLC_LOG_WARN("Update MCP server list failed (agentUuid={}): agent not found", m_lineEditUuid->text());
            ToastManager::showMessage(Toast::Type::Error, QString("更新MCP服务器列表失败 (agentUuid=%1): agent not found").arg(m_lineEditUuid->text()));
            return;
        }
    }

    for (const QString &uuid : currentAgent->mcpServers)
    {
        std::shared_ptr<McpServer> mcpServer = DataManager::getInstance()->getMcpServer(uuid);
        if (!mcpServer)
        {
            XLC_LOG_WARN("Update MCP server list failed (serverUuid={}): MCP server not found", uuid);
            continue;
        }
        QListWidgetItem *itemMcpServer = new QListWidgetItem(mcpServer->isActive ? mcpServer->name : mcpServer->name + " [未启用]", m_listWidgetMcpServers);
        itemMcpServer->setData(Qt::UserRole, QVariant::fromValue<QString>(mcpServer->uuid));
        m_listWidgetMcpServers->addItem(itemMcpServer);
    }
    m_listWidgetMcpServers->sortItems();
}

void WidgetAgentInfo::slot_onLLMsLoaded(bool success)
{
    if (!success)
        return;
    updateLLMList();
}

void WidgetAgentInfo::slot_handleStateChanged(const QVariant &data)
{
    if (data.canConvert<QJsonObject>())
    {
        QJsonObject jsonObj = data.value<QJsonObject>();
        int id = jsonObj["id"].toInt();
        QString agentUuid = jsonObj["agentUuid"].toString();

        switch (static_cast<EventBus::States>(id))
        {
        case EventBus::States::LLM_UPDATED:
        {
            updateLLMList();
            break;
        }
        case EventBus::States::MCP_SERVERS_UPDATED:
        {
            updateMCPServerList();
            break;
        }
        case EventBus::States::AGENT_UPDATED:
        {
            if (agentUuid != m_lineEditUuid->text())
                break;
            updateFormData(DataManager::getInstance()->getAgent(agentUuid));
            break;
        }
        default:
        {
            break;
        }
        }
    }
    else
    {
        XLC_LOG_ERROR("Failed to process state change event, data type exception (dataType={})", data.typeName());
    }
}

// PageSettingsLLM
PageSettingsLLM::PageSettingsLLM(QWidget *parent)
    : BaseSettingsPage<LLM, WidgetLLMInfo, DialogAddNewLLM>(
          "LLM", // 实体名称，用于日志
          []()
          { return DataManager::getInstance()->getLLMs(); },
          [](const QString &uuid)
          { return DataManager::getInstance()->getLLM(uuid); },
          [](std::shared_ptr<LLM> llm)
          { DataManager::getInstance()->addLLM(llm); },
          [](std::shared_ptr<LLM> llm)
          { DataManager::getInstance()->updateLLM(llm); },
          [](const QString &uuid)
          { DataManager::getInstance()->removeLLM(uuid); },
          [](const std::shared_ptr<LLM> &llm)
          { return llm->modelName; },
          EventBus::States::LLM_UPDATED,
          parent)
{
    connectDataManagerDataLoadedSignals();
}

void PageSettingsLLM::connectDataManagerDataLoadedSignals()
{
    connect(DataManager::getInstance(), &DataManager::sig_LLMsLoaded, this, &PageSettingsLLM::slot_onDataLoaded);
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
}

void WidgetLLMInfo::initLayout()
{
    // gLayout
    QGridLayout *gLayout = new QGridLayout(this);
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
}

void WidgetLLMInfo::updateFormData(std::shared_ptr<LLM> llm)
{
    if (!llm)
        return;
    m_lineEditUuid->setText(llm->uuid);
    m_lineEditModelID->setText(llm->modelID);
    m_lineEditModelName->setText(llm->modelName);
    m_lineEditApiKey->setText(llm->apiKey);
    m_lineEditBaseUrl->setText(llm->baseUrl);
    m_lineEditEndPoint->setText(llm->endpoint);
}

void WidgetLLMInfo::clearFormData()
{
    m_lineEditUuid->setText("");
    m_lineEditModelID->setText("");
    m_lineEditModelName->setText("");
    m_lineEditApiKey->setText("");
    m_lineEditBaseUrl->setText("");
    m_lineEditEndPoint->setText("");
}

std::shared_ptr<LLM> WidgetLLMInfo::getCurrentData()
{
    std::shared_ptr<LLM> llm = std::make_shared<LLM>();
    llm->uuid = m_lineEditUuid->text();
    llm->modelID = m_lineEditModelID->text();
    llm->modelName = m_lineEditModelName->text();
    llm->apiKey = m_lineEditApiKey->text();
    llm->baseUrl = m_lineEditBaseUrl->text();
    llm->endpoint = m_lineEditEndPoint->text();
    return llm;
}

const QString WidgetLLMInfo::getUuid()
{
    return m_lineEditUuid->text();
}

void WidgetLLMInfo::populateBasicInfo()
{
    m_lineEditUuid->setText(generateUuid());
}

// DialogAddNewLLM
DialogAddNewLLM::DialogAddNewLLM(QWidget *parent, Qt::WindowFlags f)
    : BaseDialog(parent, f)
{
    initUI();
    m_widgetLLMInfo->populateBasicInfo();
}

void DialogAddNewLLM::initWidget()
{
    setWindowTitle("新增LLM");
    resize(500, 250);
}

void DialogAddNewLLM::initItems()
{
    m_widgetLLMInfo = new WidgetLLMInfo(this);
    // m_pushButtonSave
    m_pushButtonSave = new QPushButton("保存", this);
    connect(m_pushButtonSave, &QPushButton::clicked, this, &DialogAddNewLLM::accept);
    // m_pushButtonCancel
    m_pushButtonCancel = new QPushButton("取消", this);
    connect(m_pushButtonCancel, &QPushButton::clicked, this, &DialogAddNewLLM::reject);
}

void DialogAddNewLLM::initLayout()
{
    // hLayoutButtons
    QHBoxLayout *hLayoutButtons = new QHBoxLayout();
    hLayoutButtons->addStretch();
    hLayoutButtons->addWidget(m_pushButtonSave);
    hLayoutButtons->addWidget(m_pushButtonCancel);
    // vLayout
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addWidget(m_widgetLLMInfo);
    vLayout->addStretch();
    vLayout->addLayout(hLayoutButtons);
}

std::shared_ptr<LLM> DialogAddNewLLM::getFormData()
{
    return m_widgetLLMInfo->getCurrentData();
}

// DialogAddNewAgent
DialogAddNewAgent::DialogAddNewAgent(QWidget *parent, Qt::WindowFlags f)
    : BaseDialog(parent, f)
{
    initUI();
    m_widgetAgentInfo->populateBasicInfo();
}

void DialogAddNewAgent::initWidget()
{
    setWindowTitle("新增Agent");
    resize(500, 600);
}

void DialogAddNewAgent::initItems()
{
    m_widgetAgentInfo = new WidgetAgentInfo(this);
    // m_pushButtonSave
    m_pushButtonSave = new QPushButton("保存", this);
    connect(m_pushButtonSave, &QPushButton::clicked, this, &DialogAddNewAgent::accept);
    // m_pushButtonCancel
    m_pushButtonCancel = new QPushButton("取消", this);
    connect(m_pushButtonCancel, &QPushButton::clicked, this, &DialogAddNewAgent::reject);
}

void DialogAddNewAgent::initLayout()
{
    // hLayoutButtons
    QHBoxLayout *hLayoutButtons = new QHBoxLayout();
    hLayoutButtons->addStretch();
    hLayoutButtons->addWidget(m_pushButtonSave);
    hLayoutButtons->addWidget(m_pushButtonCancel);
    // vLayout
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addWidget(m_widgetAgentInfo);
    vLayout->addStretch();
    vLayout->addLayout(hLayoutButtons);
}

std::shared_ptr<Agent> DialogAddNewAgent::getFormData()
{
    return m_widgetAgentInfo->getCurrentData();
}

// PageSettingsMcp
PageSettingsMcp::PageSettingsMcp(QWidget *parent)
    : BaseSettingsPage<McpServer, WidgetMcpServerInfo, DialogAddNewMcpServer>(
          "MCP Server",
          []()
          { return DataManager::getInstance()->getMcpServers(); },
          [](const QString &uuid)
          { return DataManager::getInstance()->getMcpServer(uuid); },
          [](std::shared_ptr<McpServer> srv)
          { DataManager::getInstance()->addMcpServer(srv); },
          [](std::shared_ptr<McpServer> srv)
          { DataManager::getInstance()->updateMcpServer(srv); },
          [](const QString &uuid)
          { DataManager::getInstance()->removeMcpServer(uuid); },
          [](const std::shared_ptr<McpServer> &srv)
          { return srv->name; },
          EventBus::States::MCP_SERVERS_UPDATED,
          parent)
{
    connectDataManagerDataLoadedSignals();
}

void PageSettingsMcp::connectDataManagerDataLoadedSignals()
{
    connect(DataManager::getInstance(), &DataManager::sig_mcpServersLoaded, this, &PageSettingsMcp::slot_onDataLoaded);
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
    // m_checkBoxIsActive
    m_checkBoxIsActive = new QCheckBox(this);
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
    m_plainTextEditEnvVars->setPlaceholderText("KEY1=value1\nKEY2=value2");
    // m_labelHost
    m_labelHost = new QLabel("Host", this);
    // m_lineEditHost
    m_lineEditHost = new QLineEdit(this);
    m_lineEditHost->setPlaceholderText("127.0.0.1");
    // m_labelPort
    m_labelPort = new QLabel("Port", this);
    // m_lineEditPort
    m_lineEditPort = new QLineEdit(this);
    m_lineEditPort->setPlaceholderText("8000");
    // m_labelBaseUrl
    m_labelBaseUrl = new QLabel("BaseUrl", this);
    // m_lineEditBaseUrl
    m_lineEditBaseUrl = new QLineEdit(this);
    m_lineEditBaseUrl->setPlaceholderText("http://localhost:8000");
    // m_labelEndpoint
    m_labelEndpoint = new QLabel("Endpoint", this);
    // m_lineEditEndpoint
    m_lineEditEndpoint = new QLineEdit(this);
    m_lineEditEndpoint->setPlaceholderText("/sse");
    // m_labelRequestHeaders
    m_labelRequestHeaders = new QLabel("请求头", this);
    // m_plainTextEditRequestHeaders
    m_plainTextEditRequestHeaders = new QPlainTextEdit(this);
    m_plainTextEditRequestHeaders->setPlaceholderText("Content-Type=application/json\nAuthorization=Bearer token");
}

void WidgetMcpServerInfo::initLayout()
{
    // gLayout
    QGridLayout *gLayout = new QGridLayout(this);
    gLayout->setContentsMargins(0, 0, 0, 0);
    gLayout->addWidget(new QLabel("启用状态", this), 0, 0);
    gLayout->addWidget(m_checkBoxIsActive, 0, 1);
    gLayout->addWidget(new QLabel("UUID", this), 1, 0);
    gLayout->addWidget(m_lineEditUuid, 1, 1);
    gLayout->addWidget(new QLabel("名称", this), 2, 0);
    gLayout->addWidget(m_lineEditName, 2, 1);
    gLayout->addWidget(new QLabel("描述", this), 3, 0);
    gLayout->addWidget(m_plainTextEditDescription, 3, 1);
    gLayout->addWidget(new QLabel("类型", this), 4, 0);
    gLayout->addWidget(m_comboBoxType, 4, 1);
    gLayout->addWidget(new QLabel("超时", this), 5, 0);
    gLayout->addWidget(m_spinBoxTimeout, 5, 1);
    gLayout->addWidget(m_labelCommand, 6, 0);
    gLayout->addWidget(m_lineEditCommand, 6, 1);
    gLayout->addWidget(m_labelArgs, 7, 0);
    gLayout->addWidget(m_plainTextEditArgs, 7, 1);
    gLayout->addWidget(m_labelEnvVars, 8, 0);
    gLayout->addWidget(m_plainTextEditEnvVars, 8, 1);
    gLayout->addWidget(m_labelHost, 9, 0);
    gLayout->addWidget(m_lineEditHost, 9, 1);
    gLayout->addWidget(m_labelPort, 10, 0);
    gLayout->addWidget(m_lineEditPort, 10, 1);
    gLayout->addWidget(m_labelBaseUrl, 11, 0);
    gLayout->addWidget(m_lineEditBaseUrl, 11, 1);
    gLayout->addWidget(m_labelEndpoint, 12, 0);
    gLayout->addWidget(m_lineEditEndpoint, 12, 1);
    gLayout->addWidget(m_labelRequestHeaders, 13, 0);
    gLayout->addWidget(m_plainTextEditRequestHeaders, 13, 1);
}

void WidgetMcpServerInfo::updateFormData(std::shared_ptr<McpServer> mcpServer)
{
    if (!mcpServer)
        return;
    m_checkBoxIsActive->setChecked(mcpServer->isActive);
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
    m_lineEditHost->setText(mcpServer->host);
    m_lineEditPort->setText(QString::number(mcpServer->port));
    m_lineEditBaseUrl->setText(mcpServer->baseUrl);
    m_lineEditEndpoint->setText(mcpServer->endpoint);
    m_plainTextEditRequestHeaders->setPlainText(mcpServer->requestHeaders);
}

void WidgetMcpServerInfo::clearFormData()
{
    m_checkBoxIsActive->setChecked(false);
    m_lineEditUuid->setText("");
    m_lineEditName->setText("");
    m_plainTextEditDescription->setPlainText("");
    m_comboBoxType->setCurrentIndex(0);
    m_spinBoxTimeout->setValue(0);
    m_lineEditCommand->setText("");
    m_plainTextEditArgs->setPlainText("");
    m_plainTextEditEnvVars->setPlainText("");
    m_lineEditHost->setText("");
    m_lineEditPort->setText("");
    m_lineEditBaseUrl->setText("");
    m_lineEditEndpoint->setText("");
    m_plainTextEditRequestHeaders->setPlainText("");
}

std::shared_ptr<McpServer> WidgetMcpServerInfo::getCurrentData()
{
    std::shared_ptr<McpServer> mcpServer = std::make_shared<McpServer>();
    mcpServer->isActive = m_checkBoxIsActive->isChecked();
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

    mcpServer->host = m_lineEditHost->text();
    mcpServer->port = m_lineEditPort->text().toInt();
    mcpServer->baseUrl = m_lineEditBaseUrl->text();
    mcpServer->endpoint = m_lineEditEndpoint->text();
    mcpServer->requestHeaders = m_plainTextEditRequestHeaders->toPlainText();

    return mcpServer;
}

const QString WidgetMcpServerInfo::getUuid()
{
    return m_lineEditUuid->text();
}

void WidgetMcpServerInfo::populateBasicInfo()
{
    m_lineEditUuid->setText(generateUuid());
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
        m_labelHost->hide();
        m_lineEditHost->hide();
        m_labelPort->hide();
        m_lineEditPort->hide();
        m_labelBaseUrl->hide();
        m_lineEditBaseUrl->hide();
        m_labelEndpoint->hide();
        m_lineEditEndpoint->hide();
        m_labelRequestHeaders->hide();
        m_plainTextEditRequestHeaders->hide();
        return;
    }
    if (index == McpServer::Type::sse || McpServer::Type::streambleHttp)
    {
        if (index == McpServer::Type::sse)
            m_lineEditEndpoint->setPlaceholderText("/sse");
        else
            m_lineEditEndpoint->setPlaceholderText("/mcp");
        m_labelCommand->hide();
        m_lineEditCommand->hide();
        m_labelArgs->hide();
        m_plainTextEditArgs->hide();
        m_labelEnvVars->hide();
        m_plainTextEditEnvVars->hide();
        m_labelHost->show();
        m_lineEditHost->show();
        m_labelPort->show();
        m_lineEditPort->show();
        m_labelBaseUrl->show();
        m_lineEditBaseUrl->show();
        m_labelEndpoint->show();
        m_lineEditEndpoint->show();
        m_labelRequestHeaders->show();
        m_plainTextEditRequestHeaders->show();
        return;
    }
    XLC_LOG_WARN("Non-existent MCP server type (type={})", m_comboBoxType->currentText());
    ToastManager::showMessage(Toast::Type::Error, QString("不存在的MCP服务器类型 (type=%1)").arg(m_comboBoxType->currentText()));
}

// DialogMountMcpServer
DialogMountMcpServer::DialogMountMcpServer(std::shared_ptr<QSet<QString>> mountedMCPServerUuidsPtr, QWidget *parent, Qt::WindowFlags f)
    : BaseDialog(parent, f), m_mountedMCPServerUuidsPtr(mountedMCPServerUuidsPtr)
{
    initUI();
}

void DialogMountMcpServer::initWidget()
{
    setWindowTitle("挂载Mcp服务器");
    resize(400, 300);
}

void DialogMountMcpServer::initItems()
{
    // m_listWidgetMcpServers
    m_listWidgetMcpServers = new QListWidget(this);
    for (const std::shared_ptr<McpServer> &mcpServer : DataManager::getInstance()->getMcpServers())
    {
        // 仅展示已挂载或已启用的服务器供用户选择
        QListWidgetItem *item;
        if (m_mountedMCPServerUuidsPtr->contains(mcpServer->uuid))
        {
            if (mcpServer->isActive)
                item = new QListWidgetItem(mcpServer->name, m_listWidgetMcpServers);
            else
                item = new QListWidgetItem(mcpServer->name + " [未启用]", m_listWidgetMcpServers);
            item->setData(Qt::UserRole, QVariant::fromValue<QString>(mcpServer->uuid));
            item->setCheckState(Qt::Checked);
        }
        else
        {
            if (mcpServer->isActive)
            {
                item = new QListWidgetItem(mcpServer->name, m_listWidgetMcpServers);
                item->setData(Qt::UserRole, QVariant::fromValue<QString>(mcpServer->uuid));
                item->setCheckState(Qt::Unchecked);
            }
            else
                continue;
        }
        m_listWidgetMcpServers->addItem(item);
    }
    m_listWidgetMcpServers->sortItems();
    connect(m_listWidgetMcpServers, &QListWidget::itemChanged, this,
            [this](QListWidgetItem *item)
            {
                const QString uuidMcpServer = item->data(Qt::UserRole).toString();
                if (item->checkState() == Qt::Checked)
                {
                    m_mountedMCPServerUuidsPtr->insert(uuidMcpServer);
                    XLC_LOG_TRACE("Mounted MCP server (uuidMcpServer={})", uuidMcpServer);
                }
                else if (item->checkState() == Qt::Unchecked)
                {
                    if (m_mountedMCPServerUuidsPtr->contains(uuidMcpServer))
                    {
                        m_mountedMCPServerUuidsPtr->remove(uuidMcpServer);
                        XLC_LOG_TRACE("Unmounted mcp server (uuid={})", uuidMcpServer);
                    }
                }
            });
    // m_pushButtonSave
    m_pushButtonSave = new QPushButton("保存", this);
    connect(m_pushButtonSave, &QPushButton::clicked, this, &DialogMountMcpServer::accept);
    // m_pushButtonCancel
    m_pushButtonCancel = new QPushButton("取消", this);
    connect(m_pushButtonCancel, &QPushButton::clicked, this, &DialogMountMcpServer::reject);
}

void DialogMountMcpServer::initLayout()
{
    // hLayoutButtons
    QHBoxLayout *hLayoutButtons = new QHBoxLayout();
    hLayoutButtons->addStretch();
    hLayoutButtons->addWidget(m_pushButtonSave);
    hLayoutButtons->addWidget(m_pushButtonCancel);
    // vLayout
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addWidget(m_listWidgetMcpServers);
    vLayout->addStretch();
    vLayout->addLayout(hLayoutButtons);
}

// DialogAddNewMcpServer
DialogAddNewMcpServer::DialogAddNewMcpServer(QWidget *parent, Qt::WindowFlags f)
    : BaseDialog(parent, f)
{
    initUI();
    m_widgetMcpServerInfo->populateBasicInfo();
}

void DialogAddNewMcpServer::initWidget()
{
    setWindowTitle("新增McpServer");
    resize(500, 700);
}

void DialogAddNewMcpServer::initItems()
{
    m_widgetMcpServerInfo = new WidgetMcpServerInfo(this);
    // m_pushButtonSave
    m_pushButtonSave = new QPushButton("保存", this);
    connect(m_pushButtonSave, &QPushButton::clicked, this, &DialogAddNewMcpServer::accept);
    // m_pushButtonCancel
    m_pushButtonCancel = new QPushButton("取消", this);
    connect(m_pushButtonCancel, &QPushButton::clicked, this, &DialogAddNewMcpServer::reject);
}

void DialogAddNewMcpServer::initLayout()
{
    // hLayoutButtons
    QHBoxLayout *hLayoutButtons = new QHBoxLayout();
    hLayoutButtons->addStretch();
    hLayoutButtons->addWidget(m_pushButtonSave);
    hLayoutButtons->addWidget(m_pushButtonCancel);
    // vLayout
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addWidget(m_widgetMcpServerInfo);
    vLayout->addStretch();
    vLayout->addLayout(hLayoutButtons);
}

std::shared_ptr<McpServer> DialogAddNewMcpServer::getFormData()
{
    return m_widgetMcpServerInfo->getCurrentData();
}

// PageSettingsStorage
PageSettingsStorage::PageSettingsStorage(QWidget *parent)
    : BaseWidget(parent)
{
    initUI();
    connect(DataManager::getInstance(), &DataManager::sig_LLMsFilePathChange, this, &PageSettingsStorage::slot_onFilePathChangedLLMs);
    connect(DataManager::getInstance(), &DataManager::sig_agentsFilePathChange, this, &PageSettingsStorage::slot_onFilePathChangedAgents);
    connect(DataManager::getInstance(), &DataManager::sig_mcpServersFilePathChange, this, &PageSettingsStorage::slot_onFilePathChangedMcpServers);
}

void PageSettingsStorage::initWidget()
{
}

void PageSettingsStorage::initItems()
{
    // m_lineEditFilePathLLMs
    m_lineEditFilePathLLMs = new QLineEdit(this);
    m_lineEditFilePathLLMs->setText(QFileInfo(DataManager::getInstance()->getFilePathLLMs()).absoluteFilePath());
    // m_pushButtonSelectFileLLMs
    m_pushButtonSelectFileLLMs = new QPushButton("选择", this);
    connect(m_pushButtonSelectFileLLMs, &QPushButton::clicked, this,
            [this]()
            {
                QString fileName = QFileDialog::getOpenFileName(this, "选择 LLMs 文件", QString(), "JSON Files (*.json);;All Files (*)");
                if (!fileName.isEmpty())
                {
                    DataManager::getInstance()->setFilePathLLMs(fileName);
                    XLC_LOG_DEBUG("Setting LLMs file path (fileName={})", fileName);
                }
            });
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
                    XLC_LOG_DEBUG("Setting agents file path (fileName={})", fileName);
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
                    XLC_LOG_DEBUG("Setting MCP server file path (fileName={})", fileName);
                }
            });
}

void PageSettingsStorage::initLayout()
{
    // gLayoutStorage
    QGridLayout *gLayoutStorage = new QGridLayout();
    gLayoutStorage->addWidget(new QLabel("LLMs", this), 0, 0);
    gLayoutStorage->addWidget(m_lineEditFilePathLLMs, 0, 1);
    gLayoutStorage->addWidget(m_pushButtonSelectFileLLMs, 0, 2);
    gLayoutStorage->addWidget(new QLabel("Agents", this), 1, 0);
    gLayoutStorage->addWidget(m_lineEditFilePathAgents, 1, 1);
    gLayoutStorage->addWidget(m_pushButtonSelectFileAgents, 1, 2);
    gLayoutStorage->addWidget(new QLabel("McpServers", this), 2, 0);
    gLayoutStorage->addWidget(m_lineEditFilePathMcpServers, 2, 1);
    gLayoutStorage->addWidget(m_pushButtonSelectFileMcpServers, 2, 2);
    // groupBoxStorage
    QGroupBox *groupBoxStorage = new QGroupBox("存储设置", this);
    groupBoxStorage->setLayout(gLayoutStorage);
    // vLayout
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addWidget(groupBoxStorage);
    vLayout->addStretch();
}

void PageSettingsStorage::slot_onFilePathChangedLLMs(const QString &filePath)
{
    if (filePath.isEmpty())
        return;
    m_lineEditFilePathLLMs->setText(QFileInfo(filePath).absoluteFilePath());
}

void PageSettingsStorage::slot_onFilePathChangedAgents(const QString &filePath)
{
    if (filePath.isEmpty())
        return;
    m_lineEditFilePathAgents->setText(QFileInfo(filePath).absoluteFilePath());
}

void PageSettingsStorage::slot_onFilePathChangedMcpServers(const QString &filePath)
{
    if (filePath.isEmpty())
        return;
    m_lineEditFilePathMcpServers->setText(QFileInfo(filePath).absoluteFilePath());
}

// PageSettingsDisplay

PageSettingsDisplay::PageSettingsDisplay(QWidget *parent)
    : BaseWidget(parent)
{
    initUI();
}

void PageSettingsDisplay::initWidget()
{
}

void PageSettingsDisplay::initItems()
{
    // m_comboBoxTheme
    m_comboBoxTheme = new QComboBox(this);
    m_comboBoxTheme->addItem("浅色");
    m_comboBoxTheme->addItem("深色");
    if (ColorRepository::s_darkMode)
        m_comboBoxTheme->setCurrentIndex(1);
    else
        m_comboBoxTheme->setCurrentIndex(0);
    // m_pushButtonTheme
    m_pushButtonTheme = new QPushButton("确定", this);
    connect(m_pushButtonTheme, &QPushButton::clicked, this,
            [this]()
            {
                QString strTheme = m_comboBoxTheme->currentText();
                if (strTheme == "浅色")
                    ColorRepository::setDarkMode(false);
                else if (strTheme == "深色")
                    ColorRepository::setDarkMode(true);
                XLC_LOG_DEBUG("Theme changed(currentTheme={})", strTheme);
            });
    // m_lineEditPrimaryColor
    m_lineEditPrimaryColor = new QLineEdit(this);
    m_lineEditPrimaryColor->setPlaceholderText("#FF5F5F");
    m_lineEditPrimaryColor->setText(ColorRepository::primaryNormal().name());
    // m_pushButtonPrimaryColor
    m_pushButtonPrimaryColor = new QPushButton("确定", this);
    connect(m_pushButtonPrimaryColor, &QPushButton::clicked, this,
            [this]()
            {
                QColor colorPrimaryNew(m_lineEditPrimaryColor->text());
                if (!colorPrimaryNew.isValid())
                    return;
                ColorRepository::setPrimaryColor(colorPrimaryNew);
                XLC_LOG_DEBUG("Primary color changed(newColor={})", m_lineEditPrimaryColor->text());
            });
}

void PageSettingsDisplay::initLayout()
{
    QGridLayout *gLayoutDisplay = new QGridLayout();
    gLayoutDisplay->addWidget(new QLabel("主题", this), 0, 0);
    gLayoutDisplay->addWidget(m_comboBoxTheme, 0, 1);
    gLayoutDisplay->addWidget(m_pushButtonTheme, 0, 2);
    gLayoutDisplay->addWidget(new QLabel("主题颜色", this), 1, 0);
    gLayoutDisplay->addWidget(m_lineEditPrimaryColor, 1, 1);
    gLayoutDisplay->addWidget(m_pushButtonPrimaryColor, 1, 2);
    // groupBoxStorage
    QGroupBox *groupBoxDisplay = new QGroupBox("显示设置", this);
    groupBoxDisplay->setLayout(gLayoutDisplay);
    // vLayout
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addWidget(groupBoxDisplay);
    vLayout->addStretch();
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
                XLC_LOG_DEBUG("跳转到GitHub");
            });
    // m_pushButtonBlog
    m_pushButtonBlog = new QPushButton("Blog", this);
    connect(m_pushButtonBlog, &QPushButton::clicked, this,
            []()
            {
                QDesktopServices::openUrl(QUrl("https://1610161.xyz"));
                XLC_LOG_DEBUG("跳转到Blog");
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
