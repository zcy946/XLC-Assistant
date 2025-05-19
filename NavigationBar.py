from BaseWidget import BaseWidget
from PySide6.QtWidgets import (
    QLabel,
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
    QRect,
    QByteArray,
)
from SvgManager import (
    svg_damoxing,
    svg_zhinengti,

)


class NavigationBar(BaseWidget):
    __items: list[dict[str, str] | dict[str, str]]
    __layout: QBoxLayout
    __colorButtonBorder: QColor
    __colorHover: QColor
    __colorFont: QColor
    __fontSize: int
    __marginL: int
    __marginT: int
    __marginR: int
    __marginB: int
    __spacing: int
    __borderRadius: int

    def __init__(self, parent=None):
        super().__init__(parent)
        self.__colorButtonBorder = QColor("#2B2D30")
        self.__color_hover = QColor("#4B4C4F")
        self.__colorFont = QColor("#DFE1E5")
        self.__fontSize = 10
        self.__marginL = 10
        self.__marginT = 10
        self.__marginR = 10
        self.__marginB = 10
        self.__spacing = 20
        self.__borderRadius = 5
        self.__items = [
            {"text": "大模型", "icon": svg_damoxing},
            {"text": "智能体", "icon": svg_zhinengti}
        ]

    def _initWidget(self):
        pass

    def _initItems(self):
        pass

    def _initLayout(self):
        self.__layout = QVBoxLayout(self)
        # self.__layout.addWidget(QLabel("NavigationBar", self))

    def paintEvent(self, event):
        painter = QPainter(self)
        font = painter.font()
        font.setPointSize(self.__fontSize)
        painter.setFont(font)
        fontMetrics = QFontMetrics(font)
        # sizeIconBg = self.width() - self.__marginL - self.__marginR
        sizeIconBg = self.width() - self.__marginL - self.__marginR - 10
        sizeIcon = sizeIconBg // 5 * 3
        for i in range(0, len(self.__items)):
            text = self.__items[i].get("text", "undefined")
            svgCode = self.__items[i].get("icon", "")
            textWidth = fontMetrics.horizontalAdvance(text)
            textHeight = fontMetrics.height()
            xIconBg = (self.width() - sizeIconBg) // 2
            yIconBg = self.__marginT + i * (sizeIconBg + self.__spacing + textHeight)
            xIcon = xIconBg + (sizeIconBg - sizeIcon) // 2
            yIcon = yIconBg + (sizeIconBg - sizeIcon) // 2
            xText = (self.width() - textWidth) // 2
            # yText = (i + 1) * (sizeIconBg + self.__spacing + textHeight)
            yText = yIconBg + sizeIconBg + textHeight
            # 绘制边框
            painter.setPen(self.__colorButtonBorder)
            painter.drawRoundedRect(xIconBg, yIconBg, sizeIconBg, sizeIconBg, self.__borderRadius, self.__borderRadius)
            # 绘制图标
            self.__drawIcon(painter, svgCode, xIcon, yIcon, sizeIcon, sizeIcon)
            # 绘制文字
            painter.setPen(QColor(self.__colorFont))
            painter.drawText(xText, yText, text)
        painter.end()

    def __drawIcon(self, painter: QPainter, svgCode: str, x: int, y: int, w: int, h: int) -> None:
        # 将 SVG 字符串转换为 QByteArray
        svg_bytes = QByteArray(svgCode.encode('utf-8'))
        svg_renderer = QSvgRenderer(svg_bytes, self)
        # 渲染 SVG 图标
        svg_renderer.render(painter, QRect(x, y, w, h))
