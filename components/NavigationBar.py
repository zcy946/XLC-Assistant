from BaseWidget import BaseWidget
from PySide6.QtWidgets import (
    QWidget,
    QBoxLayout,
    QVBoxLayout
)
from PySide6.QtSvg import QSvgRenderer
from PySide6.QtGui import (
    QPainter,
    QColor,
    QFontMetrics,
)
from PySide6.QtCore import (
    Qt,
    QRect,
    QByteArray, Signal,
)
from loguru import logger

KEY_TEXT = "text"
KEY_ICON = "icon"
COLOR_ITEM_HOVER = COLOR_ITEM_SELECTED = "#4B4C4F"
COLOR_FONT = "#DFE1E5"

class NavigationBar(BaseWidget):
    signal_index_changed = Signal(int)
    __layout: QBoxLayout
    __color_item_hover: QColor
    __color_font: QColor
    __font_size: int
    __margin_left: int
    __margin_top: int
    __margin_right: int
    __margin_bottom: int
    __spacing: int
    __border_radius: int
    __size_icon_background: int
    __size_icon: int
    __height_text: int
    __items: list[dict[str, str] | dict[str, str]]
    __index_hover: int
    __index_pressed: int

    def __init__(self, parent:QWidget | None = None):
        super().__init__(parent)
        self.__color_item_hover = QColor(COLOR_ITEM_HOVER)
        self.__color_font = QColor(COLOR_FONT)
        self.__font_size = 10
        self.__margin_left = 10
        self.__margin_top = 10
        self.__margin_right = 10
        self.__margin_bottom = 10
        self.__spacing = 20
        self.__border_radius = 5
        self.__size_icon_background = 0
        self.__height_text = 0
        self.__items = []
        self.__index_hover = -1
        self.__index_pressed = 0

    def _init_widget(self):
        self.setMouseTracking(True)

    def _init_items(self):
        pass

    def _init_layout(self):
        self.__layout = QVBoxLayout(self)

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        font = painter.font()
        font.setPointSize(self.__font_size)
        painter.setFont(font)
        fontMetrics = QFontMetrics(font)
        self.__size_icon_background = self.width() - self.__margin_left - self.__margin_right - 10
        size_icon = self.__size_icon_background // 5 * 3
        for i in range(0, len(self.__items)):
            text = self.__items[i].get(KEY_TEXT, "undefined")
            svg_code = self.__items[i].get(KEY_ICON, "")
            width_text = fontMetrics.horizontalAdvance(text)
            self.__height_text = fontMetrics.height()
            x_icon_background = (self.width() - self.__size_icon_background) // 2
            y_icon_background = self.__margin_top + i * (self.__size_icon_background + self.__spacing +  self.__height_text)
            x_icon = x_icon_background + (self.__size_icon_background - size_icon) // 2
            y_icon = y_icon_background + (self.__size_icon_background - size_icon) // 2
            x_text = (self.width() - width_text) // 2
            y_text = y_icon_background + self.__size_icon_background +  self.__height_text
            # 绘制边框
            if (i == self.__index_hover or i == self.__index_pressed):
                painter.setPen(Qt.NoPen)
                painter.setBrush(self.__color_item_hover)
                painter.drawRoundedRect(x_icon_background, y_icon_background, self.__size_icon_background, self.__size_icon_background,
                                        self.__border_radius, self.__border_radius)
            # 绘制图标
            painter.setBrush(Qt.NoBrush)
            self.__draw_icon(painter, svg_code, x_icon, y_icon, size_icon, size_icon)
            # 绘制文字
            painter.setPen(QColor(self.__color_font))
            painter.drawText(x_text, y_text, text)
        painter.end()

    def mouseMoveEvent(self, event):
        index_current_hover = (event.pos().y() - self.__margin_top) // (self.__size_icon_background + self.__height_text + self.__spacing)
        if index_current_hover >= 0 and index_current_hover + 1 <= len(self.__items) and self.__index_hover != index_current_hover:
            self.__index_hover = index_current_hover
            # logger.debug("hover: {}", self.__index_hover)
            self.update()

    def mousePressEvent(self, event):
        if event.button() == Qt.MouseButton.LeftButton:
            index_current_selected = (event.pos().y() - self.__margin_top) // (self.__size_icon_background + self.__height_text + self.__spacing)
            if index_current_selected >= 0 and index_current_selected + 1 <= len(self.__items) and self.__index_pressed != index_current_selected:
                self.__index_pressed = index_current_selected
                # logger.debug("pressed: {}", self.__index_pressed)
                self.signal_index_changed.emit(self.__index_pressed)
                self.update()

    def leaveEvent(self, event):
        self.__index_hover = -1
        self.update()

    def __draw_icon(self, painter: QPainter, svg_code: str, x: int, y: int, w: int, h: int) -> None:
        # 将 SVG 字符串转换为 QByteArray
        svg_bytes = QByteArray(svg_code.encode('utf-8'))
        svg_renderer = QSvgRenderer(svg_bytes, self)
        # 渲染 SVG 图标
        svg_renderer.render(painter, QRect(x, y, w, h))

    def add_svg_item(self, text: str, svg_code: str):
        """
        添加SVG项
        参数：
            text: str - 要展示的文本
            svg_code: str - 要展示svg的代码
        """
        self.__items.append({KEY_TEXT: text, KEY_ICON: svg_code})
        self.update()