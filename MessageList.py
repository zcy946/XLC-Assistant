from PySide6.QtWidgets import (
    QListWidget,
)


class MessageList(QListWidget):
    def __init__(self, parent: None):
        super().__init__(parent)

    def __init_widget(self):
        pass