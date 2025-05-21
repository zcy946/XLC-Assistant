from BaseWidget import BaseWidget
from PySide6.QtWidgets import (
    QWidget,
    QLabel,
    QVBoxLayout,
)


class PageAgent(BaseWidget):
    def __init__(self, parent: QWidget | None = None):
        super().__init__(parent)

    def _init_widget(self):
        pass

    def _init_items(self):
        pass

    def _init_layout(self):
        v_layout = QVBoxLayout(self)
        v_layout.setContentsMargins(0, 0, 0, 0)
        v_layout.addWidget(QLabel("PageAgent", self))
