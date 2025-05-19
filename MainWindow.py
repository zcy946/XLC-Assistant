from BaseWidget import BaseWidget
from PySide6.QtWidgets import (
    QVBoxLayout,
    QSplitter
)
from NavigationBar import NavigationBar
from PageContainer import PageContainer


class MainWindow(BaseWidget):
    __splitter: QSplitter
    __navigationBar: NavigationBar
    __pageContainer: PageContainer

    def __init__(self, parent=None):
        super().__init__(parent)

    def _initWidget(self) -> None:
        """初始化窗口"""
        self.setWindowTitle("ShallowSeek")
        self.resize(800, 600)

    def _initItems(self) -> None:
        """初始化控件"""
        # __navigationBar
        self.__navigationBar = NavigationBar(self)
        self.__navigationBar.setMinimumWidth(70)
        self.__navigationBar.setMaximumWidth(80)
        # __pageContainer
        self.__pageContainer = PageContainer(self)
        # __splitter
        self.__splitter = QSplitter(self)
        # 防止子控件被压扁
        self.__splitter.setChildrenCollapsible(False)
        self.__splitter.addWidget(self.__navigationBar)
        self.__splitter.addWidget(self.__pageContainer)
        self.__splitter.setStretchFactor(0, 1)
        self.__splitter.setStretchFactor(1, 9)

    def _initLayout(self) -> None:
        """初始化布局"""
        vLayout: QVBoxLayout = QVBoxLayout(self)
        vLayout.setSpacing(0)
        vLayout.setContentsMargins(0, 0, 0, 0)
        vLayout.addWidget(self.__splitter)
