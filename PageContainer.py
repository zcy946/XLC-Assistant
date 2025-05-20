from BaseWidget import BaseWidget
from PySide6.QtWidgets import (
    QLabel,
    QStackedLayout

)
from PageChat import PageChat
from PageAgent import PageAgent


class PageContainer(BaseWidget):
    __stacked_layout: QStackedLayout
    __page_llm: PageChat
    __page_agent: PageAgent

    def __init__(self, parent=None):
        super().__init__(parent)

    def _init_widget(self):
        pass

    def _init_items(self):
        # __page_llm
        self.__page_llm = PageChat(self)
        # ____page_agent
        self.____page_agent = PageAgent(self)

    def _init_layout(self):
        self.__stacked_layout = QStackedLayout(self)
        self.__stacked_layout.setContentsMargins(0, 0, 0, 0)
        self.__stacked_layout.addWidget(self.__page_llm)
        self.__stacked_layout.addWidget(self.____page_agent)

    def setCurrentIndex(self, index: int) -> None:
        self.__stacked_layout.setCurrentIndex(index)