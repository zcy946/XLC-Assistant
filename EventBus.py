from PySide6.QtCore import (
    QObject, Signal
)
from enum import Enum, auto
from typing import Any
from threading import Lock
from loguru import logger


class SingletonMeta(type(QObject)):
    _instances = {}
    _lock = Lock()

    def __call__(cls, *args, **kwargs):
        if cls not in cls._instances:
            with cls._lock:
                if cls not in cls._instances:
                    cls._instances[cls] = super().__call__(*args, **kwargs)
        return cls._instances[cls]

class EventBus(QObject, metaclass=SingletonMeta):
    # 信号
    signal_button_clicked = Signal(Any) # 按钮点击
    signal_state_changed = Signal(Any) # 状态改变
    signal_message_sent = Signal(Any) # 发送消息
    signal_message_received = Signal(Any) # 消息响应
    # 按钮id
    class Buttons(Enum):
        CLEAR_CONTEXT = auto()
    # 事件类型
    class EventType(Enum):
        ButtonClicked = auto()
        StateChanged = auto()
        MessageSent = auto()
        MessageReceived = auto()

    def __init__(self, parent=None):
        super().__init__(parent=parent)
        self._event_signal_map = {
            self.EventType.ButtonClicked: self.signal_button_clicked,
            self.EventType.StateChanged: self.signal_state_changed,
            self.EventType.MessageSent: self.signal_message_sent,
            self.EventType.MessageReceived: self.signal_message_received
        }

    def publish(self, event_type: "EventBus.EventType", data: Any = None) -> None:
        """发布事件"""
        if not isinstance(event_type, self.EventType):
            logger.error("Invalid event type: {}", event_type)
            return

        logger.debug("[event] -> {}:{}:[{}]", event_type, type(data), data)

        signal = self._event_signal_map.get(event_type)
        if signal:
            signal.emit(data)
        else:
            logger.error("Unhandled event type: {}", event_type)



