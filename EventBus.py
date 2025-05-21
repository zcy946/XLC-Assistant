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
    signal_send_message = Signal(Any) # 发送消息
    class EventType(Enum):
        ButtonClicked = auto()
        SendMessage = auto()

    def __init__(self, parent=None):
        super().__init__(parent=parent)

    def publish(self, event_type: "EventBus.EventType", data: Any) -> None:
        logger.debug("[event] -> {}:{}:[{}]", event_type, type(data), data)
        match event_type:
            case EventBus.EventType.ButtonClicked:
                self.signal_button_clicked.emit(data)
            case EventBus.EventType.SendMessage:
                self.signal_send_message.emit(data)
            case _:
                logger.error("[event] -> unknow event type:{}:[{}]", type(data), data)



