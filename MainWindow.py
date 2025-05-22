from BaseWidget import BaseWidget
from PySide6.QtWidgets import (
    QWidget,
    QVBoxLayout,
    QSplitter
)
from components.NavigationBar import NavigationBar
from PageContainer import PageContainer
from SvgManager import (
    svg_chat,
    svg_agent,
    svg_setting,
)
from loguru import logger
from LLMService import LLMService
from LLMController import LLMController


class MainWindow(BaseWidget):
    __splitter: QSplitter
    __navigation_bar: NavigationBar
    __page_container: PageContainer

    def __init__(self, parent=None):
        super().__init__(parent)
        # 初始化llm控制器
        LLMController()

    def _init_widget(self) -> None:
        """初始化窗口"""
        self.setWindowTitle("XLC Assistant")
        self.resize(800, 600)

    def _init_items(self) -> None:
        """初始化控件"""
        # __navigation_bar
        self.__navigation_bar = NavigationBar(self)
        self.__navigation_bar.setMinimumWidth(70)
        self.__navigation_bar.setMaximumWidth(80)
        self.__navigation_bar.add_item_svg("对话", svg_chat)
        self.__navigation_bar.add_item_svg("智能体", svg_agent)
        self.__navigation_bar.add_item_svg("设置", svg_setting)
        self.__navigation_bar.add_non_selectable_item("设置")
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
        logger.debug(self.__navigation_bar.get_index("设置"))
        if index == self.__navigation_bar.get_index("设置"):
            self.__open_setting_dialog()
        logger.debug("current page index: {}", index)
        self.__page_container.setCurrentIndex(index)

    def __open_setting_dialog(self):
        logger.debug("打开设置对话框")