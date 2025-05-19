from BaseWidget import BaseWidget
from PySide6.QtWidgets import (
    QLabel,
    QStackedLayout

)
from AssistantPage import AssistantPage


class PageContainer(BaseWidget):
    __stackedLayout: QStackedLayout
    __assistantPage: AssistantPage

    def __init__(self, parent=None):
        super().__init__(parent)

    def _initWidget(self):
        pass

    def _initItems(self):
        self.__assistantPage = AssistantPage(self)

    def _initLayout(self):
        self.__stackedLayout = QStackedLayout(self)
        self.__stackedLayout.setContentsMargins(0, 0, 0, 0)
        self.__stackedLayout.addWidget(self.__assistantPage)
