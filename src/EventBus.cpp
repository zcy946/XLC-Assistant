#include "eventbus.h"
#include <QDebug>

EventBus::EventBus(QObject *parent) : QObject(parent) {}

void EventBus::publish(EventType eventType, const QVariant &data)
{
    qDebug() << "[event] ->" << static_cast<int>(eventType) << ":" << data.typeName() << ":[" << data << "]";

    switch (eventType)
    {
    case EventType::ButtonClicked:
        emit signalButtonClicked(data);
        break;
    case EventType::StateChanged:
        emit signalStateChanged(data);
        break;
    case EventType::MessageSent:
        emit signalMessageSent(data);
        break;
    case EventType::MessageReceived:
        emit signalMessageReceived(data);
        break;
    default:
        qWarning() << "Unhandled event type:" << static_cast<int>(eventType);
        break;
    }
}