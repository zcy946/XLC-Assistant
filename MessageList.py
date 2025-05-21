from PySide6.QtWidgets import (
    QListWidget,
    QWidget,
)


class MessageList(QListWidget):
    def __init__(self, parent: QWidget | None = None):
        super().__init__(parent)

    def __init_widget(self):
        pass
