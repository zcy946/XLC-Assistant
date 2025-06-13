#include "PageSettings.h"
#include <QLabel>

PageSettings::PageSettings(QWidget *parent)
    : BaseWidget(parent)
{
    initUI();
}

void PageSettings::initWidget()
{
    QLabel *label = new QLabel("PageSettings", this);
}

void PageSettings::initItems()
{
}

void PageSettings::initLayout()
{
}
