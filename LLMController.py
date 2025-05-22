from EventBus import EventBus
import asyncio
from LLMService import LLMService
from loguru import logger
from PySide6.QtCore import QObject
from threading import Lock


class SingletonMeta(type(QObject)):
    _instances = {}
    _lock = Lock()

    def __call__(cls, *args, **kwargs):
        if cls not in cls._instances:
            with cls._lock:
                if cls not in cls._instances:
                    cls._instances[cls] = super().__call__(*args, **kwargs)
        return cls._instances[cls]


class LLMController(QObject, metaclass=SingletonMeta):
    def __init__(self, parent: QObject | None = None):
        super().__init__(parent=parent)

    def init(self):
        self.__init_handlers()

    def __init_handlers(self):
        EventBus().signal_message_sent.connect(self.__on_message_sent)

    def __on_message_sent(self, user_input: str):
        """处理用户的消息请求"""
        if not user_input:
            return
        asyncio.ensure_future(self.__execute_task(user_input))

    async def __execute_task(self, user_input: str):
        """运行任务并处理结果"""
        try:
            # 获取结果
            result = await LLMService().execute_task(user_input)
            # 发送结果到 UI
            EventBus().publish(EventBus.EventType.MessageReceived, result)
        except Exception as e:
            logger.error(f"Task {user_input} failed: {e}")
            EventBus().publish(EventBus.EventType.MessageReceived, f"Error: {str(e)}")
