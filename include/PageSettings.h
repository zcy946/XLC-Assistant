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

class PageSettingsAgent;
class PageSettingsMcp;
class PageSettingsData;
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

private:
    void showAgentInfo(const QString &uuid);
};

class WidgetAgentInfo : public BaseWidget
{
    Q_OBJECT
public:
    explicit WidgetAgentInfo(QWidget *parent = nullptr);
    ~WidgetAgentInfo() = default;
    /**
     * 更新表单数据
     */
    void updateData(std::shared_ptr<Agent> agent);

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;

private:
    QLineEdit *m_lineEditUuid;
    QLineEdit *m_lineEditName;
    QSpinBox *m_spinBoxChildren;
    QPlainTextEdit *m_plainTextEditDescription;
    QLineEdit *m_lineEditModelName;
    QSpinBox *m_spinBoxContext;
    QDoubleSpinBox *m_doubleSpinBoxTemperature;
    QDoubleSpinBox *m_doubleSpinBoxTopP;
    QSpinBox *m_spinBoxMaxTokens;
    QPlainTextEdit *m_plainTextEditSystemPrompt;
    QListWidget *m_listWidgetMcpServers;
    QPushButton *m_pushButtonReset;
    QPushButton *m_pushButtonSave;
};

class PageSettingsMcp : public BaseWidget
{
    Q_OBJECT
private Q_SLOTS:
    void slot_onListWidgetItemClicked(QListWidgetItem *item);

public:
    explicit PageSettingsMcp(QWidget *parent = nullptr);

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;

private:
    QListWidget *m_listWidgetMcpServers;

private:
    void showMcpServerInfo(const QString &uuid);
};

class PageSettingsData : public BaseWidget
{
    Q_OBJECT
public:
    explicit PageSettingsData(QWidget *parent = nullptr);

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;

private:
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
};

#endif // PAGESETTINGS_H