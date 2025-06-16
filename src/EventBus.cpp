#include "eventbus.h"
#include <QDebug>

EventBus::EventBus(QObject *parent) : QObject(parent) {}

void EventBus::publish(EventType eventType, const QVariant &data)
{
    qDebug() << "[event] ->" << static_cast<int>(eventType) << ":" << data.typeName() << ":[" << data << "]";

    switch (eventType)
    {
    case EventType::ButtonClicked:
        Q_EMIT sig_buttonClicked(data);
        break;
    case EventType::StateChanged:
        Q_EMIT sig_stateChanged(data);
        break;
    case EventType::MessageSent:
        Q_EMIT sig_messageSent(data);
        break;
    case EventType::MessageReceived:
        Q_EMIT sig_messageReceived(data);
        break;
    default:
        qWarning() << "Unhandled event type:" << static_cast<int>(eventType);
        break;
    }
}