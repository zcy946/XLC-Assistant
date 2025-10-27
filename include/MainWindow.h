#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "BaseWidget.hpp"
#include "XlcNavigationBar.h"
#include <QStackedLayout>
#include "PageChat.h"
#include "PageSettings.h"
#include <QResizeEvent>

class MainWindow : public BaseWidget
{
    Q_OBJECT
private Q_SLOTS:
    void handleNavigationBarItemChanged(const QString &targetId);
    void handlePageSwitched(const QVariant &data);

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;

private:
    XlcNavigationBar *m_navigationBar;
    QStackedLayout *m_stackedLayout;
    PageChat *m_pageChat;
    PageSettings *m_pageSettings;
    QMap<QString, QWidget *> m_pages; // targetId - QWidget *
};

#endif // MAINWINDOW_H