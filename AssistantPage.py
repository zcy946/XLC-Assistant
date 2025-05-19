from BaseWidget import BaseWidget
from PySide6.QtWidgets import (
    QLabel,
    QVBoxLayout,
)


class AssistantPage(BaseWidget):
    def __init__(self, parent=None):
        super().__init__(parent)

    def _initWidget(self):
        pass

    def _initItems(self):
        pass

    def _initLayout(self):
        vLayout = QVBoxLayout(self)
        vLayout.setContentsMargins(0, 0, 0, 0)
        vLayout.addWidget(QLabel("AssistantPage", self))
