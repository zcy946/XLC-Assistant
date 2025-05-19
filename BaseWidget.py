from PySide6.QtWidgets import QWidget
from random import randint
from PySide6.QtGui import (
    QPainter,
    QColor
)


class BaseWidget(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.__initUI()

    def __initUI(self):
        self._initWidget()
        self._initItems()
        self._initLayout()

    def _initWidget(self):
        """子类可重写"""
        pass

    def _initItems(self):
        """子类可重写"""
        pass

    def _initLayout(self):
        """子类可重写"""
        pass

    def paintEvent(self, event):
        """开发调试使用"""
        painter = QPainter(self)
        painter.setPen(QColor(f"#{randint(0, 0xFFFFFF):06x}"))
        painter.drawRect(self.rect().adjusted(1, 1, -1, -1))
        painter.end()