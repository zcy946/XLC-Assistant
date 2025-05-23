from BaseWidget import BaseWidget
from PySide6.QtWidgets import (
    QWidget,
    QVBoxLayout,
    QSplitter,
)
from ListChat import ListChat
from CentralWidgetChat import CentralWidgetChat


class PageChat(BaseWidget):
    __splitter: QSplitter
    __list_chat: ListChat
    __list_message: CentralWidgetChat

    def __init__(self, parent: QWidget | None = None):
        super().__init__(parent)

    def _init_widget(self):
        pass

    def _init_items(self):
        # __list_chat
        self.__list_chat = ListChat(self)
        # __list_message
        self.__list_message = CentralWidgetChat(self)
        # ____splitter
        self.__splitter = QSplitter(self)
        self.__splitter.setChildrenCollapsible(False)
        self.__splitter.addWidget(self.__list_chat)
        self.__splitter.addWidget(self.__list_message)
        self.__splitter.setStretchFactor(0, 2)
        self.__splitter.setStretchFactor(1, 8)

    def _init_layout(self):
        v_layout = QVBoxLayout(self)
        v_layout.setContentsMargins(0, 0, 0, 0)
        v_layout.addSpacing(0)
        v_layout.addWidget(self.__splitter)
