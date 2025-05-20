from BaseWidget import BaseWidget
from PySide6.QtWidgets import (
    QVBoxLayout,
    QSplitter
)
from components.NavigationBar import NavigationBar
from PageContainer import PageContainer
from SvgManager import (
    svg_damoxing,
    svg_zhinengti,
)
from loguru import logger


class MainWindow(BaseWidget):
    __splitter: QSplitter
    __navigation_bar: NavigationBar
    __page_container: PageContainer

    def __init__(self, parent=None):
        super().__init__(parent)

    def _init_widget(self) -> None:
        """初始化窗口"""
        self.setWindowTitle("ShallowSeek")
        self.resize(800, 600)

    def _init_items(self) -> None:
        """初始化控件"""
        # __navigation_bar
        self.__navigation_bar = NavigationBar(self)
        self.__navigation_bar.setMinimumWidth(70)
        self.__navigation_bar.setMaximumWidth(80)
        self.__navigation_bar.add_svg_item("大模型", svg_damoxing)
        self.__navigation_bar.add_svg_item("智能体", svg_zhinengti)
        # __page_container
        self.__page_container = PageContainer(self)
        self.__navigation_bar.signal_index_changed.connect(self.__handle_navigation)

        # __splitter
        self.__splitter = QSplitter(self)
        # 防止子控件被压扁
        self.__splitter.setChildrenCollapsible(False)
        self.__splitter.addWidget(self.__navigation_bar)
        self.__splitter.addWidget(self.__page_container)
        self.__splitter.setStretchFactor(0, 1)
        self.__splitter.setStretchFactor(1, 9)

    def _init_layout(self) -> None:
        """初始化布局"""
        v_layout: QVBoxLayout = QVBoxLayout(self)
        v_layout.setSpacing(0)
        v_layout.setContentsMargins(0, 0, 0, 0)
        v_layout.addWidget(self.__splitter)

    def __handle_navigation(self, index: int) -> None:
        """处理导航栏点击事件"""
        logger.debug("current page index: {}", index)
        self.__page_container.setCurrentIndex(index)