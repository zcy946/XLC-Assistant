#ifndef PAGESETTINGS_H
#define PAGESETTINGS_H

#include "BaseWidget.hpp"
#include <QListWidget>
#include <QStackedWidget>
#include <QMap>
#include "DataManager.h"
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QMenu>
#include "BaseDialog.hpp"

class PageSettingsLLM;
class PageSettingsAgent;
class PageSettingsMcp;
class PageSettingsStorage;
class PageAbout;
class PageSettings : public BaseWidget
{
    Q_OBJECT
public:
    explicit PageSettings(QWidget *parent = nullptr);
    /**
     * 添加配置页
     */
    void addPage(const QString &name, QWidget *page);

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;

private:
    QListWidget *m_listWidget;
    QStackedWidget *m_stackedWidget;
    QMap<QString, QWidget *> m_pages;
};

class WidgetLLMInfo;
class PageSettingsLLM : public BaseWidget
{
    Q_OBJECT
private Q_SLOTS:
    void slot_onListWidgetItemClicked(QListWidgetItem *item);
    void slot_onLLMsLoaded(bool success);

public:
    explicit PageSettingsLLM(QWidget *parent = nullptr);
    ~PageSettingsLLM() = default;

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;

private:
    QListWidget *m_listWidgetLLMs;
    WidgetLLMInfo *m_widgetLLMInfo;
    QPushButton *m_pushButtonAdd;
    QPushButton *m_pushButtonReset;
    QPushButton *m_pushButtonSave;

private:
    void showLLMInfo(const QString &uuid);
};

class WidgetLLMInfo : public BaseWidget
{
    Q_OBJECT
public:
    explicit WidgetLLMInfo(QWidget *parent = nullptr);
    ~WidgetLLMInfo() = default;
    /**
     * 更新表单数据
     */
    void updateData(std::shared_ptr<LLM> llm);
    /**
     * 获取当前表单数据
     */
    std::shared_ptr<LLM> getCurrentData();
    const QString getUuid();
    void populateBasicInfo();

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;

private:
    QLineEdit *m_lineEditUuid;
    QLineEdit *m_lineEditModelID;
    QLineEdit *m_lineEditModelName;
    QLineEdit *m_lineEditApiKey;
    QLineEdit *m_lineEditBaseUrl;
    QLineEdit *m_lineEditEndPoint;
};

class DialogAddNewLLM : public BaseDialog
{
    Q_OBJECT
public:
    DialogAddNewLLM(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~DialogAddNewLLM() = default;

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;

private:
    WidgetLLMInfo *m_widgetLLMInfo;
    QPushButton *m_pushButtonSave;
    QPushButton *m_pushButtonCancel;
};

class WidgetAgentInfo;
class PageSettingsAgent : public BaseWidget
{
    Q_OBJECT
private Q_SLOTS:
    void slot_onListWidgetItemClicked(QListWidgetItem *item);
    void slot_onAgentsOrMcpServersLoaded(bool success);

public:
    explicit PageSettingsAgent(QWidget *parent = nullptr);

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;

private:
    QListWidget *m_listWidgetAgents;
    WidgetAgentInfo *m_widgetAgentInfo;
    QPushButton *m_pushButtonAdd;
    QPushButton *m_pushButtonReset;
    QPushButton *m_pushButtonSave;

private:
    void showAgentInfo(const QString &uuid);
};

class WidgetAgentInfo : public BaseWidget
{
    Q_OBJECT
private Q_SLOTS:
    void slot_onLLMsLoaded(bool success);

public:
    explicit WidgetAgentInfo(QWidget *parent = nullptr);
    ~WidgetAgentInfo() = default;
    /**
     * 更新表单数据
     */
    void updateData(std::shared_ptr<Agent> agent);
    /**
     * 获取当前表单数据
     */
    std::shared_ptr<Agent> getCurrentData();
    const QString getUuid();
    void populateBasicInfo();

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;

private:
    QLineEdit *m_lineEditUuid;
    QLineEdit *m_lineEditName;
    QPlainTextEdit *m_plainTextEditDescription;
    QComboBox *m_comboBoxLLM;
    QSpinBox *m_spinBoxContext;
    QDoubleSpinBox *m_doubleSpinBoxTemperature;
    QDoubleSpinBox *m_doubleSpinBoxTopP;
    QSpinBox *m_spinBoxMaxTokens;
    QPlainTextEdit *m_plainTextEditSystemPrompt;
    QListWidget *m_listWidgetMcpServers;
    QMenu *m_contextMenuMcpServers;
    QListWidget *m_listWidgetConversations;
    QMenu *m_contextMenuConversations;
};

class DialogAddNewAgent : public BaseDialog
{
    Q_OBJECT
public:
    DialogAddNewAgent(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~DialogAddNewAgent() = default;

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;

private:
    WidgetAgentInfo *m_widgetAgentInfo;
    QPushButton *m_pushButtonSave;
    QPushButton *m_pushButtonCancel;
};

class WidgetMcpServerInfo;
class PageSettingsMcp : public BaseWidget
{
    Q_OBJECT
private Q_SLOTS:
    void slot_onListWidgetItemClicked(QListWidgetItem *item);
    void slot_onMcpServersLoaded(bool success);

public:
    explicit PageSettingsMcp(QWidget *parent = nullptr);

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;

private:
    QListWidget *m_listWidgetMcpServers;
    WidgetMcpServerInfo *m_widgetMcpServerInfo;
    QPushButton *m_pushButtonAdd;
    QPushButton *m_pushButtonReset;
    QPushButton *m_pushButtonSave;

private:
    void showMcpServerInfo(const QString &uuid);
};

class WidgetMcpServerInfo : public BaseWidget
{
    Q_OBJECT
private Q_SLOTS:
    void slot_onComboBoxCurrentIndexChanged(int index);

public:
    explicit WidgetMcpServerInfo(QWidget *parent = nullptr);
    ~WidgetMcpServerInfo() = default;
    /**
     * 更新表单数据
     */
    void updateData(std::shared_ptr<McpServer> mcpServer);
    /**
     * 获取当前表单数据
     */
    std::shared_ptr<McpServer> getCurrentData();
    const QString getUuid();
    void populateBasicInfo();

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;

private:
    QLineEdit *m_lineEditUuid;
    QLineEdit *m_lineEditName;
    QPlainTextEdit *m_plainTextEditDescription;
    QComboBox *m_comboBoxType;
    QSpinBox *m_spinBoxTimeout;
    QLabel *m_labelCommand;
    QLineEdit *m_lineEditCommand;
    QLabel *m_labelArgs;
    QPlainTextEdit *m_plainTextEditArgs;
    QLabel *m_labelEnvVars;
    QPlainTextEdit *m_plainTextEditEnvVars;
    QLabel *m_labelHost;
    QLineEdit *m_lineEditHost;
    QLabel *m_labelPort;
    QLineEdit *m_lineEditPort;
    QLabel *m_labelBaseUrl;
    QLineEdit *m_lineEditBaseUrl;
    QLabel *m_labelEndpoint;
    QLineEdit *m_lineEditEndpoint;
    QLabel *m_labelRequestHeaders;
    QPlainTextEdit *m_plainTextEditRequestHeaders;
};

class DialogMountMcpServer : public BaseDialog
{
    Q_OBJECT
public:
    DialogMountMcpServer(std::shared_ptr<QSet<QString>> uuidsMcpServer, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~DialogMountMcpServer() = default;

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;

private:
    std::shared_ptr<QSet<QString>> m_uuidsMcpServer;
    QListWidget *m_listWidgetMcpServers;
    QPushButton *m_pushButtonSave;
    QPushButton *m_pushButtonCancel;
};

class DialogAddNewMcpServer : public BaseDialog
{
    Q_OBJECT
public:
    DialogAddNewMcpServer(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~DialogAddNewMcpServer() = default;

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;

private:
    WidgetMcpServerInfo *m_widgetMcpServerInfo;
    QPushButton *m_pushButtonSave;
    QPushButton *m_pushButtonCancel;
};

class PageSettingsStorage : public BaseWidget
{
    Q_OBJECT
private Q_SLOTS:
    void slot_onFilePathChangedLLMs(const QString &filePath);
    void slot_onFilePathChangedAgents(const QString &filePath);
    void slot_onFilePathChangedMcpServers(const QString &filePath);

public:
    explicit PageSettingsStorage(QWidget *parent = nullptr);

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;

private:
    QLineEdit *m_lineEditFilePathLLMs;
    QPushButton *m_pushButtonSelectFileLLMs;
    QLineEdit *m_lineEditFilePathAgents;
    QPushButton *m_pushButtonSelectFileAgents;
    QLineEdit *m_lineEditFilePathMcpServers;
    QPushButton *m_pushButtonSelectFileMcpServers;
};

class PageAbout : public BaseWidget
{
    Q_OBJECT
public:
    explicit PageAbout(QWidget *parent = nullptr);

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;

private:
    QPushButton *m_pushButtonGitHub;
    QPushButton *m_pushButtonBlog;
};

#endif // PAGESETTINGS_H