from BaseWidget import BaseWidget
from PySide6.QtWidgets import (
    QLabel,
    QStackedLayout

)
from AssistantPage import AssistantPage


class PageContainer(BaseWidget):
    __stacked_layout: QStackedLayout
    __page_assistant: AssistantPage

    def __init__(self, parent=None):
        super().__init__(parent)

    def _init_widget(self):
        pass

    def _init_items(self):
        self.__page_assistant = AssistantPage(self)

    def _init_layout(self):
        self.__stacked_layout = QStackedLayout(self)
        self.__stacked_layout.setContentsMargins(0, 0, 0, 0)
        self.__stacked_layout.addWidget(self.__page_assistant)
