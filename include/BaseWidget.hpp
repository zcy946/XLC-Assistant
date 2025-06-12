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
        int r = QRandomGenerator::global()->bounded(256);
        int g = QRandomGenerator::global()->bounded(256);
        int b = QRandomGenerator::global()->bounded(256);
        QPainter painter(this);
        painter.setPen(QColor(r, g, b));
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