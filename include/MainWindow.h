#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "BaseWidget.hpp"
#include "CNavigationBar.h"
#include <QStackedLayout>
#include "PageChat.h"
#include "PageSettings.h"

class MainWindow : public BaseWidget
{
    Q_OBJECT
private Q_SLOTS:
    void handleNavigationBarIndexChanged(int index, const QString &text);
    void handlePageSwitched(const QVariant &data);

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;

private:
    CNavigationBar *m_navigationBar;
    QStackedLayout *m_stackedLayout;
    PageChat *m_pageChat;
    PageSettings *m_pageSettings;
};

#endif // MAINWINDOW_H