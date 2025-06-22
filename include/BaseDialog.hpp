#ifndef BASEDIALOG_H
#define BASEDIALOG_H

#include <QDialog>
#include <QRandomGenerator>
#include <QPainter>

class BaseDialog : public QDialog
{
public:
    explicit BaseDialog(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags()) : QDialog(parent, f) {};
    virtual ~BaseDialog() = default;

protected:
    void paintEvent(QPaintEvent *event) override
    {
#ifdef QT_DEBUG
        QPainter painter(this);
        painter.setPen(QColor::fromRgb(QRandomGenerator::global()->generate()));
        painter.drawRect(rect().adjusted(1, 1, -1, -1));
        painter.end();
        QWidget::paintEvent(event);
#endif
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

#endif // BASEDIALOG_H