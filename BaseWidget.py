from PySide6.QtWidgets import QWidget
from random import randint
from PySide6.QtGui import (
    QPainter,
    QColor
)


class BaseWidget(QWidget):
    def __init__(self, parent: QWidget | None = None):
        super().__init__(parent)
        self.__initUI()

    def __initUI(self):
        self._init_widget()
        self._init_items()
        self._init_layout()

    def _init_widget(self):
        """子类可重写"""
        pass

    def _init_items(self):
        """子类可重写"""
        pass

    def _init_layout(self):
        """子类可重写"""
        pass

    def paintEvent(self, event):
        """开发调试使用"""
        painter = QPainter(self)
        painter.setPen(QColor(f"#{randint(0, 0xFFFFFF):06x}"))
        # painter.drawRect(self.rect().adjusted(1, 1, -1, -1))
        painter.end()

    # NOTE
    # 主题切换思路：
    #   1. 写一个切换主题的空实现虚函数，
    #   2. 将主题管理单例类的主题切换信号连接到此函数
    #   3. 这样所有继承自此类的子类只需要重写此函数即可实现主题的切换
