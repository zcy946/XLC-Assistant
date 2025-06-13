#ifndef BASEWIDGET_H
#define BASEWIDGET_H

#include <QWidget>
#include <QRandomGenerator>
#include <QPainter>

class BaseWidget : public QWidget
{
public:
    explicit BaseWidget(QWidget *parent = nullptr) : QWidget(parent) {};
    virtual ~BaseWidget() = default;

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QPainter painter(this);
        painter.setPen(QColor::fromRgb(QRandomGenerator::global()->generate()));
        painter.drawRect(rect().adjusted(1, 1, -1, -1));
        painter.end();
        QWidget::paintEvent(event);
    }

protected:
    virtual void initWidget() {};
    virtual void initItems() {};
    virtual void initLayout() {};
    void initUI()
    {
        initWidget();
        initItems();
        initLayout();
    }
};

#endif // BaseWidget