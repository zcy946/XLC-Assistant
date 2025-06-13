#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "BaseWidget.hpp"
#include "CNavigationBar.h"
#include <QSplitter>

class MainWindow : public BaseWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;

private:
    CNavigationBar *m_navigationBar;
};

#endif // MAINWINDOW_H