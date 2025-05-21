from PySide6.QtWidgets import (
    QWidget,
    QPlainTextEdit
)


class UserInputEdit(QPlainTextEdit):
    def __init__(self, parent: QWidget | None = None):
        super().__init__(parent)

    def __init_widget(self):
        pass
