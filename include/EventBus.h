#ifndef EVENTBUS_H
#define EVENTBUS_H

#include <QObject>
#include <QVariant>
#include "Singleton.h"

class EventBus : public QObject, public Singleton<EventBus>
{
    Q_OBJECT
    friend class Singleton<EventBus>;

public:
    // Button IDs
    enum class Buttons
    {
        CLEAR_CONTEXT,
        UPDATE_SYSTEM_PROMPT,
        RESET_MODEL_ARGS
    };

    // State IDs
    enum class States
    {
        MODEL_UPDATED,
        MCP_SERVERS_UPDATED
    };

    // Event Types
    enum class EventType
    {
        ButtonClicked,
        StateChanged,
        MessageSent,
        MessageReceived
    };
    Q_ENUM(EventType)

    void publish(EventType eventType, const QVariant &data = QVariant());

signals:
    void sig_buttonClicked(const QVariant &data);
    void sig_stateChanged(const QVariant &data);
    void sig_messageSent(const QVariant &data);
    void sig_messageReceived(const QVariant &data);

private:
    explicit EventBus(QObject *parent = nullptr);
};

#endif // EVENTBUS_H