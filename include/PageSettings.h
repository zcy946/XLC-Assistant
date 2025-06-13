#ifndef PAGESETTINGS_H
#define PAGESETTINGS_H

#include "BaseWidget.hpp"

class PageSettings : public BaseWidget
{
    Q_OBJECT
public:
    explicit PageSettings(QWidget *parent = nullptr);

protected:
    void initWidget() override;
    void initItems() override;
    void initLayout() override;
};

#endif // PAGESETTINGS_H