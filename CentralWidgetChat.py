from BaseWidget import BaseWidget
from PySide6.QtWidgets import (
    QVBoxLayout,
    QLabel,
    QSplitter,
)
from PySide6.QtCore import (
    Qt,
)
from MessageList import MessageList
from UserInputEdit import UserInputEdit


class CentralWidgetChat(BaseWidget):
    __splitter: QSplitter
    __message_list: MessageList
    __plaintext_edit: UserInputEdit
    def __init__(self, parent=None):
        super().__init__(parent)

    def _init_widget(self):
        pass

    def _init_items(self):
        # __message_list
        self.__message_list = MessageList(self)
        # __plaintext_edit
        self.__plaintext_edit = UserInputEdit(self)
        # __splitter
        self.__splitter = QSplitter(self)
        self.__splitter.setOrientation(Qt.Orientation.Vertical)
        self.__splitter.addWidget(self.__message_list)
        # NOTE 将用户输入下方加入按钮，并用垂直布局包裹再加入__splitter
        self.__splitter.addWidget(self.__plaintext_edit)
        self.__splitter.setStretchFactor(0, 8)
        self.__splitter.setStretchFactor(0, 2)

    def _init_layout(self):
        v_layout = QVBoxLayout(self)
        v_layout.setContentsMargins(0, 0, 0, 0)
        v_layout.addWidget(self.__splitter)
