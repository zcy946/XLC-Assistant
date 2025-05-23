from BaseWidget import BaseWidget
from PySide6.QtWidgets import (
    QVBoxLayout,
    QLabel,
)
from PySide6.QtGui import (
    QPainter,
    QColor,
)

COLOR_BACKGROUND = "#2D2D2D"
COLOR_BORDER = "#3C3C3C"


class ListChat(BaseWidget):
    def __init__(self, parent=None):
        super().__init__(parent)

    def _init_widget(self):
        pass

    def _init_items(self):
        pass

    def _init_layout(self):
        v_layout = QVBoxLayout(self)
        v_layout.setContentsMargins(0, 0, 0, 0)
        v_layout.addWidget(QLabel("ListChat", self))

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setPen(QColor(COLOR_BORDER))
        rect = self.rect().adjusted(1, 1, -1, -1)
        painter.drawLine(rect.width(), rect.y(), rect.width(), rect.height())
        painter.end()
