from EventBus import EventBus
import asyncio
from LLMService import LLMService
from loguru import logger
from PySide6.QtCore import QObject
from threading import Lock
from ConfigManager import ConfigManager # Import ConfigManager


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
    __llm_service: LLMService
    __message_history: list | None

    def __init__(self, parent: QObject | None = None):
        super().__init__(parent=parent)
        self.__llm_service = LLMService()
        self.__message_history = list()
        self.__init_handlers()
        # 订阅配置改变事件
        EventBus().signal_config_event.connect(self.__on_config_event)
        logger.info("LLMController initialized and subscribed to config events.")


    def __init_handlers(self):
        EventBus().signal_message_sent.connect(self.__on_message_sent)
        EventBus().signal_button_clicked.connect(self.__on_button_clicked_clear_history)

    def __on_button_clicked_clear_history(self, data):
        """清除上下文"""
        if data["id"] == EventBus.Buttons.CLEAR_CONTEXT:
            self.__message_history.clear()
            logger.info("Message history has been cleared by LLMController.")


    def __on_config_event(self, event_payload: dict):
        event_id = event_payload.get("id")
        data = event_payload.get("data")
        config_manager = ConfigManager()

        if event_id == EventBus.ConfigEvents.AGENT_TEMPLATE_UPDATED:
            updated_template_name = data.get("template_name")
            current_default_name = config_manager.get_current_app_config().default_config_name
            
            # If the LLMService is currently using the template that got updated
            # (which is typically the default template), then reload.
            if updated_template_name == current_default_name:
                logger.info(
                    f"Default agent template '{updated_template_name}' was updated. "
                    f"Instructing LLMService to reload configuration."
                )
                self.__llm_service.update_agent_config(updated_template_name)
            else:
                logger.info(
                    f"Agent template '{updated_template_name}' was updated, but it's not the current default ('{current_default_name}'). "
                    f"LLMService not reloaded for this change."
                )

        elif event_id == EventBus.ConfigEvents.DEFAULT_AGENT_TEMPLATE_CHANGED:
            new_default_template_name = data.get("template_name")
            logger.info(
                f"Default agent template changed to '{new_default_template_name}'. "
                f"Instructing LLMService to use new default configuration."
            )
            self.__llm_service.update_agent_config(new_default_template_name)


    def __on_message_sent(self, user_input: str):
        """处理用户的消息请求"""
        if not user_input:
            return
        asyncio.ensure_future(self.__execute_task(user_input))

    async def __execute_task(self, user_input: str):
        """运行任务并处理结果"""
        try:
            result, self.__message_history = await self.__llm_service.execute_task(user_input, self.__message_history)
            EventBus().publish(EventBus.EventType.MessageReceived, result)
        except Exception as e:
            logger.error(f"Task {user_input} failed: {e}")
            EventBus().publish(EventBus.EventType.MessageReceived, f"Error: {str(e)}")