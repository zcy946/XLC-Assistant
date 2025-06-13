#ifndef BORDERDEBUGGER_H
#define BORDERDEBUGGER_H

#include <QObject>
#include <QWidget>
#include <QEvent>
#include <QPainter>
#include <QRandomGenerator>

class BorderDebugger : public QObject
{
public:
    BorderDebugger(QWidget *target, QObject *parent = nullptr)
        : QObject(parent), m_target(target)
    {
        if (m_target)
        {
#ifdef QT_DEBUG
            m_target->installEventFilter(this);
#endif
        }
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() == QEvent::Paint && watched == m_target)
        {
            QPainter painter(static_cast<QWidget *>(watched));
            painter.setPen(QColor::fromRgb(QRandomGenerator::global()->generate()));
            painter.drawRect(static_cast<QWidget *>(watched)->rect().adjusted(1, 1, -1, -1));
        }
        return QObject::eventFilter(watched, event);
    }

private:
    QWidget *m_target;
};

#endif // BORDERDEBUGGER_H