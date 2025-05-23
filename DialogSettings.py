from PySide6.QtWidgets import (
    QDialog,
    QWidget,
    QListWidget,
    QListWidgetItem,
    QSplitter,
    QStackedLayout,
    QVBoxLayout,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QPlainTextEdit,
    QPushButton,
    QComboBox,
    QSlider,
    QCheckBox,
    QSpinBox,
    QDoubleSpinBox,
)
from PySide6.QtCore import (
    Qt,
    Signal,
)
from BaseWidget import BaseWidget
from EventBus import EventBus
from loguru import logger


class PageSystemPrompt(BaseWidget):
    __lineedit_agent_name: QLineEdit
    __plaintext_edit_system_prompt: QPlainTextEdit
    __pushbutton_save: QPushButton

    def __init__(self, parent: QWidget | None = None):
        super().__init__(parent)

    def _init_widget(self):
        pass

    def _init_items(self):
        # __lineedit_agent_name
        self.__lineedit_agent_name = QLineEdit(self)
        self.__lineedit_agent_name.setPlaceholderText("智能体名称")
        # __plaintext_edit_system_prompt
        self.__plaintext_edit_system_prompt = QPlainTextEdit(self)
        self.__plaintext_edit_system_prompt.setPlaceholderText("智能体提示词")
        # __pushbutton_save
        self.__pushbutton_save = QPushButton(self)
        self.__pushbutton_save.setText("保存")
        self.__pushbutton_save.clicked.connect(self.__on_pushbutton_save_clicked)

    def _init_layout(self):
        v_layout = QVBoxLayout(self)
        v_layout.addWidget(QLabel("名称", self))
        v_layout.addWidget(self.__lineedit_agent_name)
        v_layout.addWidget(QLabel("提示词", self))
        v_layout.addWidget(self.__plaintext_edit_system_prompt)
        v_layout.addWidget(self.__pushbutton_save, 0, Qt.AlignmentFlag.AlignRight)

    def __on_pushbutton_save_clicked(self):
        EventBus().publish(EventBus.EventType.ButtonClicked, {
            "id": EventBus.Buttons.UPDATE_SYSTEM_PROMPT,
            "message": "Update system promot",
            "data": {
                "agent_name": self.__lineedit_agent_name.text(),
                "system_prompt": self.__plaintext_edit_system_prompt.toPlainText()
            }
        })


class PageModel(BaseWidget):
    __combobox_model: QComboBox
    __slider_model_temperature: QSlider
    __double_spinbox_model_temperature: QDoubleSpinBox
    __slider_model_top_p: QSlider
    __double_spinbox_model_top_p: QDoubleSpinBox
    __slider_model_contexts_number: QSlider
    __spinbox_model_contexts_number: QSpinBox
    __checkbox_max_tokens_number: QCheckBox
    __spinbox_max_tokens_number: QSpinBox
    __checkbox_streaming_output: QCheckBox
    __pushbutton_reset: QPushButton

    def __init__(self, parent: QWidget | None = None):
        super().__init__(parent)

    def _init_widget(self):
        pass

    def _init_items(self):
        # __combobox_model
        self.__combobox_model = QComboBox(self)
        self.__combobox_model.addItem("DeepSeek-V3")
        self.__combobox_model.currentTextChanged.connect(self.__update_model)
        # __slider_model_temperature
        self.__slider_model_temperature = QSlider(self)
        self.__slider_model_temperature.setOrientation(Qt.Orientation.Horizontal)
        self.__slider_model_temperature.setRange(0, 200)  # 放大100倍，取值时再缩回来
        self.__slider_model_temperature.valueChanged.connect(self.__on_slider_model_temperature_value_changed)
        # __double_spinbox_model_temperature
        self.__double_spinbox_model_temperature = QDoubleSpinBox(self)
        self.__double_spinbox_model_temperature.setRange(0, 2)
        self.__double_spinbox_model_temperature.valueChanged.connect(
            self.__on_double_spinbox_model_temperature_value_changed)
        # __slider_model_top_p
        self.__slider_model_top_p = QSlider(self)
        self.__slider_model_top_p.setOrientation(Qt.Orientation.Horizontal)
        self.__slider_model_top_p.setRange(0, 100)
        self.__slider_model_top_p.valueChanged.connect(self.__on_slider_model_top_p_value_changed)
        # __double_spinbox_model_top_p
        self.__double_spinbox_model_top_p = QDoubleSpinBox(self)
        self.__double_spinbox_model_top_p.setRange(0, 1)
        self.__double_spinbox_model_top_p.valueChanged.connect(self.__on_double_spinbox_model_top_p_value_changed)
        # __slider_contexts_number
        self.__slider_model_contexts_number = QSlider(self)
        self.__slider_model_contexts_number.setRange(0, 100)
        self.__slider_model_contexts_number.setOrientation(Qt.Orientation.Horizontal)
        self.__slider_model_contexts_number.valueChanged.connect(self.__on_slider_model_contexts_number_value_changed)
        # __spinbox_model_context_number
        self.__spinbox_model_contexts_number = QSpinBox(self)
        self.__spinbox_model_contexts_number.setRange(0, 100)
        self.__spinbox_model_contexts_number.valueChanged.connect(self.__on_spinbox_model_contexts_number_value_changed)
        # __checkbox_max_tokens_number
        self.__checkbox_max_tokens_number = QCheckBox(self)
        self.__checkbox_max_tokens_number.setText("最大Token数")
        self.__checkbox_max_tokens_number.checkStateChanged.connect(
            self.__on_checkbox_max_tokens_number_check_state_changed)
        # __spinbox_max_tokens_number
        self.__spinbox_max_tokens_number = QSpinBox(self)
        self.__spinbox_max_tokens_number.setRange(0, 10000000)
        self.__spinbox_max_tokens_number.hide()
        self.__spinbox_max_tokens_number.valueChanged.connect(self.__update_model)
        # __checkbox_streaming_output
        self.__checkbox_streaming_output = QCheckBox(self)
        self.__checkbox_streaming_output.setText("流式输出")
        self.__checkbox_streaming_output.checkStateChanged.connect(self.__update_model)
        # __pushbutton_reset
        self.__pushbutton_reset = QPushButton(self)
        self.__pushbutton_reset.setText("重置")
        self.__pushbutton_reset.clicked.connect(self.__on_pushbutton_reset_clicked)

    def _init_layout(self):
        # h_layout_model_temperature
        h_layout_model_temperature = QHBoxLayout()
        h_layout_model_temperature.addWidget(self.__slider_model_temperature, 8)
        h_layout_model_temperature.addWidget(self.__double_spinbox_model_temperature, 2)
        # h_layout_model_top_p
        h_layout_model_top_p = QHBoxLayout()
        h_layout_model_top_p.addWidget(self.__slider_model_top_p, 8)
        h_layout_model_top_p.addWidget(self.__double_spinbox_model_top_p, 2)
        # h_layout_model_contexts_number
        h_layout_model_contexts_number = QHBoxLayout()
        h_layout_model_contexts_number.addWidget(self.__slider_model_contexts_number, 8)
        h_layout_model_contexts_number.addWidget(self.__spinbox_model_contexts_number, 2)
        # v_layout
        v_layout = QVBoxLayout(self)
        v_layout.addWidget(QLabel("默认模型", self))
        v_layout.addWidget(self.__combobox_model, 0, Qt.AlignmentFlag.AlignLeft)
        v_layout.addWidget(QLabel("模型温度", self))
        v_layout.addLayout(h_layout_model_temperature)
        v_layout.addWidget(QLabel("Top-P", self))
        v_layout.addLayout(h_layout_model_top_p)
        v_layout.addWidget(QLabel("上下文数", self))
        v_layout.addLayout(h_layout_model_contexts_number)
        v_layout.addWidget(self.__checkbox_max_tokens_number)
        v_layout.addWidget(self.__spinbox_max_tokens_number)
        v_layout.addWidget(self.__checkbox_streaming_output)
        v_layout.addWidget(self.__pushbutton_reset, 0, Qt.AlignmentFlag.AlignRight)
        v_layout.addStretch()

    def __update_model(self, model: str | None = None,
                       temperature: float | None = None,
                       top_p: float | None = None,
                       contexts_number: int | None = None,
                       max_tokens: int | None = None,
                       streaming_output: bool | None = None):
        EventBus().publish(EventBus.EventType.StateChanged, {
            "id": EventBus.States.MODEL_UPDATED,
            "message": "Update model",
            "data": {
                "model": self.__combobox_model.currentText() if model is None else model,
                "temperature": self.__double_spinbox_model_temperature.value() if temperature is None else temperature,
                "top_p": self.__double_spinbox_model_top_p.value() if top_p is None else top_p,
                "contexts_number": self.__spinbox_model_contexts_number.value() if contexts_number is None else contexts_number,
                "max_tokens": self.__spinbox_max_tokens_number.value() if max_tokens is None else max_tokens,
                "streaming_output": self.__checkbox_streaming_output.isChecked() if streaming_output is None else streaming_output
            }
        })

    def __on_slider_model_temperature_value_changed(self, value: int):
        """模型温度滑动条槽函数"""
        self.__double_spinbox_model_temperature.setValue(value / 100)  # 为了视觉效果放大了100倍，使用时缩小100倍

    def __on_double_spinbox_model_temperature_value_changed(self, value: float):
        self.__slider_model_temperature.setValue(int(value * 100))
        self.__update_model()

    def __on_slider_model_top_p_value_changed(self, value: int):
        """模型top-p滑动条槽函数"""
        self.__double_spinbox_model_top_p.setValue(value / 100)

    def __on_double_spinbox_model_top_p_value_changed(self, value: float):
        self.__slider_model_top_p.setValue(int(value * 100))
        self.__update_model()

    def __on_slider_model_contexts_number_value_changed(self, value: int):
        """模型上下文数量滑动条槽函数"""
        self.__spinbox_model_contexts_number.setValue(value)
        self.__update_model()

    def __on_spinbox_model_contexts_number_value_changed(self, value: int):
        self.__slider_model_contexts_number.setValue(value)

    def __on_checkbox_max_tokens_number_check_state_changed(self, state: Qt.CheckState):
        """最大token数开关槽函数"""
        self.__spinbox_max_tokens_number.show() if state == Qt.CheckState.Checked else self.__spinbox_max_tokens_number.hide()

    def __on_pushbutton_reset_clicked(self):
        """重置按钮槽函数"""
        logger.debug("__on_pushbutton_reset_clicked")
        EventBus().publish(EventBus.EventType.ButtonClicked, {
            "id": EventBus.Buttons.RESET_MODEL_ARGS,
            "message": "Reset model's args"
        })


class PageMcpServer(BaseWidget):
    class WidgetMcpServer(BaseWidget):
        signal_check_state_changed = Signal(str, Qt.CheckState)
        __label_name: QLabel
        __label_description: QLabel
        __label_url: QLabel
        __checkbox: QCheckBox
        __name: str
        __description: str
        __url: str

        def __init__(self, server_id: str, name: str, description: str, url: str, parent: QWidget | None = None):
            self.__id = server_id
            self.__name = name
            self.__description = description
            self.__url = url
            super().__init__(parent)

        def _init_widget(self):
            logger.debug(123456)
            pass

        def _init_items(self):
            # __label_name
            self.__label_name = QLabel(self)
            self.__label_name.setText(self.__name)
            # __label_description
            self.__label_description = QLabel(self)
            self.__label_description.setText(self.__description)
            # __label_url
            self.__label_url = QLabel(self)
            self.__label_url.setText(self.__url)
            # __checkbox
            self.__checkbox = QCheckBox(self)
            self.__checkbox.checkStateChanged.connect(self.__on_checkbox_check_state_changed)

        def _init_layout(self):
            # v_layout_details
            v_layout_details = QVBoxLayout()
            v_layout_details.setContentsMargins(0, 0, 0, 0)
            v_layout_details.addWidget(self.__label_name)
            v_layout_details.addWidget(self.__label_description)
            v_layout_details.addWidget(self.__label_url)
            # h_layout
            h_layout = QHBoxLayout(self)
            h_layout.addLayout(v_layout_details)
            h_layout.addWidget(self.__checkbox, 0, Qt.AlignmentFlag.AlignRight)

        def __on_checkbox_check_state_changed(self, state: Qt.CheckState):
            self.signal_check_state_changed.emit(self.__id, state)

    __list_widget: QListWidget
    __label_state_selected: QLabel
    __mcp_servers: list[dict[str, str]]
    __selected_mcp_servers: list[dict[str, str]]

    def __init__(self, parent: QWidget | None = None):
        self.__mcp_servers = []  # Note 通过事件总线获取
        self.__selected_mcp_servers = []
        super().__init__(parent)
        # TEST
        self.add_item("test-uuid-01", "测试服务器1name", "测试服务器1description", "测试服务器1url")
        self.add_item("test-uuid-02", "测试服务器2name", "测试服务器2description", "测试服务器2url")

    def _init_widget(self):
        pass

    def _init_items(self):
        # __list_widget
        self.__list_widget = QListWidget(self)

    def _init_layout(self):
        v_layout = QVBoxLayout(self)
        v_layout.addWidget(QLabel("MCP设置", self))
        v_layout.addWidget(self.__list_widget)

    def add_item(self, id: str, name: str, description: str, url: str):
        """向列表中添加mcp服务器"""
        item = QListWidgetItem(self.__list_widget)
        widget_mcp_server = PageMcpServer.WidgetMcpServer(id, name, description, url, self.__list_widget)
        widget_mcp_server.signal_check_state_changed.connect(self.__on_item_mcp_selected)
        # 设置 size hint，确保 item 有高度
        item.setSizeHint(widget_mcp_server.sizeHint())
        self.__list_widget.addItem(item)
        self.__list_widget.setItemWidget(item, widget_mcp_server)

    def __on_item_mcp_selected(self, server_id: str, state: Qt.CheckState):
        """mcp服务器被选中槽函数"""
        logger.debug(f"append mcp server: {server_id}") \
            if (
                state == Qt.CheckState.Checked) \
            else (
            logger.debug(f"remove mcp server: {server_id}")
        )
        if state == Qt.CheckState.Checked:
            # 添加服务器
            self.__selected_mcp_servers.append({
                "id": server_id
            })
        else:
            # 删除服务器
            for i, item in enumerate(self.__selected_mcp_servers[:]):
                if item["id"] == server_id:
                    self.__selected_mcp_servers.pop(i)
                    break
        EventBus().publish(EventBus.EventType.StateChanged, {
            "id": EventBus.States.MCP_SERVERS_UPDATED,
            "message": "MCP Servers updated",
            "data": {
                "mcp_servers": self.__selected_mcp_servers
            }
        })


class SettingsCentralWidget(BaseWidget):
    __stacked_layout: QStackedLayout
    __page_system_prompt: PageSystemPrompt
    __page_model: PageModel
    __page_mcp_server: PageMcpServer

    def __init__(self, parent: QWidget | None = None):
        super().__init__(parent)

    def _init_widget(self):
        pass

    def _init_items(self):
        # __page_system_prompt
        self.__page_system_prompt = PageSystemPrompt(self)
        # __page_model
        self.__page_model = PageModel(self)
        # __page_mcp_server
        self.__page_mcp_server = PageMcpServer(self)

    def _init_layout(self):
        self.__stacked_layout = QStackedLayout(self)
        self.__stacked_layout.setContentsMargins(0, 0, 0, 0)
        self.__stacked_layout.addWidget(self.__page_system_prompt)
        self.__stacked_layout.addWidget(self.__page_model)
        self.__stacked_layout.addWidget(self.__page_mcp_server)

    def set_current_index(self, index: int):
        if index < 0 or index > self.__stacked_layout.count():
            return
        self.__stacked_layout.setCurrentIndex(index)


class DialogSettings(QDialog):
    __splitter: QSplitter
    __list_widget: QListWidget
    __settings_central_widget: SettingsCentralWidget

    def __init__(self, parent: QWidget | None = None):
        super().__init__(parent)
        self._init_widget()
        self._init_items()
        self._init_layout()

    def _init_widget(self):
        self.setWindowTitle("设置")
        self.setModal(True)

    def _init_items(self):
        # __list_widget
        self.__list_widget = QListWidget(self)
        self.__list_widget.addItem("提示词设置")
        self.__list_widget.addItem("模型设置")
        self.__list_widget.addItem("MCP服务器")
        self.__list_widget.currentItemChanged.connect(self.__on_selection_changed)
        # __settings_central_widget
        self.__settings_central_widget = SettingsCentralWidget(self)
        # __splitter
        self.__splitter = QSplitter(self)
        self.__splitter.setChildrenCollapsible(False)
        self.__splitter.addWidget(self.__list_widget)
        self.__splitter.addWidget(self.__settings_central_widget)
        self.__splitter.setStretchFactor(0, 2)
        self.__splitter.setStretchFactor(1, 8)

    def _init_layout(self):
        v_layout = QVBoxLayout(self)
        v_layout.addSpacing(0)
        v_layout.addWidget(self.__splitter)

    def showEvent(self, event):
        # TODO 在此处获取配置信息，更新各个变量
        logger.debug("settings dialog showed")
        super().showEvent(event)

    def closeEvent(self, event):
        # TODO 将system_prompt、model、mcp服务器信息存储在当前窗口中，当关闭时再触发更新
        logger.debug("settings dialog closed/hid")
        super().closeEvent(event)

    def __on_selection_changed(self):
        self.__settings_central_widget.set_current_index(self.__list_widget.currentIndex().row())
