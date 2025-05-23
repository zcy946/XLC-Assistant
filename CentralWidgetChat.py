from BaseWidget import BaseWidget
from PySide6.QtWidgets import (
    QVBoxLayout,
    QLabel,
    QSplitter,
    QWidget,
    QPushButton,
    QHBoxLayout,
)
from PySide6.QtCore import (
    Qt,
)
from MessageList import MessageList
from UserInputEdit import UserInputEdit
from EventBus import EventBus
from typing import Any, Tuple
from loguru import logger


class CentralWidgetChat(BaseWidget):
    __splitter: QSplitter
    __message_list: MessageList
    __widget_text_area: QWidget
    __plaintext_edit: UserInputEdit
    __pushbutton_clear_message: QPushButton
    __pushbutton_clear_context: QPushButton
    __pushbutton_send: QPushButton

    def __init__(self, parent: QWidget | None = None):
        super().__init__(parent)

    def _init_widget(self):
        pass

    def _init_items(self):
        # __message_list
        self.__message_list = MessageList(self)
        EventBus().signal_message_sent.connect(self.__on_message_sent)
        EventBus().signal_message_received.connect(self.__on_message_received)
        # __widget_text_area
        self.__widget_text_area = QWidget(self)
        # __plaintext_edit
        self.__plaintext_edit = UserInputEdit(self.__widget_text_area)
        self.__plaintext_edit.setPlaceholderText("输入消息，开始聊天")
        # __pushbutton_clear_message
        self.__pushbutton_clear_message = QPushButton(self)
        self.__pushbutton_clear_message.setText("清空消息")
        self.__pushbutton_clear_message.clicked.connect(self.__on_pushbutton_clear_message_clicked)
        # __pushbutton_clear_context
        self.__pushbutton_clear_context = QPushButton(self)
        self.__pushbutton_clear_context.setText("清除上下文")
        self.__pushbutton_clear_context.clicked.connect(self.__on_pushbutton_clear_context_clicked)
        # __pushbutton_send
        self.__pushbutton_send = QPushButton(self)
        self.__pushbutton_send.setText("发送")
        self.__pushbutton_send.clicked.connect(self.__on_pushbutton_send_clicked)
        # __splitter
        self.__splitter = QSplitter(self)
        self.__splitter.setChildrenCollapsible(False)
        self.__splitter.setOrientation(Qt.Orientation.Vertical)
        self.__splitter.addWidget(self.__message_list)
        self.__splitter.addWidget(self.__widget_text_area)
        self.__splitter.setStretchFactor(0, 8)
        self.__splitter.setStretchFactor(1, 2)

    def _init_layout(self):
        # h_layout_tools
        h_layout_tools = QHBoxLayout()
        h_layout_tools.setContentsMargins(0, 0, 0, 0)
        h_layout_tools.addWidget(self.__pushbutton_clear_message)
        h_layout_tools.addWidget(self.__pushbutton_clear_context)
        h_layout_tools.addStretch()
        h_layout_tools.addWidget(self.__pushbutton_send)
        # v_layout_text_area
        v_layout_text_area = QVBoxLayout(self.__widget_text_area)
        v_layout_text_area.setContentsMargins(0, 0, 0, 0)
        v_layout_text_area.addWidget(self.__plaintext_edit)
        v_layout_text_area.addLayout(h_layout_tools)
        # v_layout
        v_layout = QVBoxLayout(self)
        # v_layout.setContentsMargins(0, 0, 0, 0)
        v_layout.addWidget(self.__splitter)

    def __on_pushbutton_send_clicked(self):
        """发送按钮槽函数"""
        user_input = self.__plaintext_edit.toPlainText().strip()
        if not user_input:
            return
        self.__plaintext_edit.clear()
        EventBus().publish(EventBus.EventType.MessageSent, user_input)

    def __on_message_sent(self, text: Any):
        self.__message_list.addItem(f"Q: {text}")

    def __on_message_received(self, text: Any):
        self.__message_list.addItem(f"A: {text}")

    def __on_pushbutton_clear_message_clicked(self):
        self.__message_list.clear()

    def __on_pushbutton_clear_context_clicked(self):
        EventBus().publish(EventBus.EventType.ButtonClicked, {
            "id": EventBus.Buttons.CLEAR_CONTEXT,
            "message": "Clear context"
        })
        self.__message_list.addItem("----------已清除上下文----------")
