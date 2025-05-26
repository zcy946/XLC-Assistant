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
    QDialogButtonBox,
)
from PySide6.QtCore import (
    Qt,
    Signal,
)
from BaseWidget import BaseWidget
from EventBus import EventBus
from loguru import logger
from ConfigManager import ConfigManager, AgentConfig, MCPServer
from typing import Dict, Any, List, Optional


class TempAgentSettings:
    def __init__(self):
        self.agent_name: Optional[str] = None
        self.system_prompt: Optional[str] = None
        self.model: Optional[str] = None
        self.temperature: Optional[float] = None
        self.top_p: Optional[float] = None
        self.context_size: Optional[int] = None
        self.max_tokens: Optional[int] = None
        self.enable_streaming: Optional[bool] = None
        self.mounted_mcp_server_ids: Optional[List[str]] = None

    def update_from_dict(self, data: Dict[str, Any]):
        for key, value in data.items():
            if hasattr(self, key):
                setattr(self, key, value)

    def get_changed_fields(self, original_config: AgentConfig) -> Dict[str, Any]:
        changed = {}
        if self.system_prompt is not None and self.system_prompt != original_config.system_prompt:
            changed['system_prompt'] = self.system_prompt
        if self.model is not None and self.model != original_config.model:
            changed['model'] = self.model
        if self.temperature is not None and self.temperature != original_config.temperature:
            changed['temperature'] = self.temperature
        if self.top_p is not None and self.top_p != original_config.top_p:
            changed['top_p'] = self.top_p
        if self.context_size is not None and self.context_size != original_config.context_size:
            changed['context_size'] = self.context_size
        if self.max_tokens != original_config.max_tokens:
            changed['max_tokens'] = self.max_tokens
        if self.enable_streaming is not None and self.enable_streaming != original_config.enable_streaming:
            changed['enable_streaming'] = self.enable_streaming
        if self.mounted_mcp_server_ids is not None and \
           set(self.mounted_mcp_server_ids) != set(original_config.mounted_mcp_server_ids):
            changed['mounted_mcp_server_ids'] = self.mounted_mcp_server_ids
        return changed


class PageSystemPrompt(BaseWidget):
    __lineedit_agent_name: QLineEdit
    __plaintext_edit_system_prompt: QPlainTextEdit
    settings_changed = Signal(dict)

    def __init__(self, parent: QWidget | None = None):
        super().__init__(parent)

    def _init_widget(self): pass

    def _init_items(self):
        self.__lineedit_agent_name = QLineEdit(self)
        self.__lineedit_agent_name.setPlaceholderText("智能体名称 (当前为默认模板)")
        self.__lineedit_agent_name.setReadOnly(True)
        self.__plaintext_edit_system_prompt = QPlainTextEdit(self)
        self.__plaintext_edit_system_prompt.setPlaceholderText("智能体提示词")
        self.__plaintext_edit_system_prompt.textChanged.connect(
            self._on_prompt_changed)

    def _init_layout(self):
        v_layout = QVBoxLayout(self)
        v_layout.addWidget(QLabel("当前编辑模板名称", self))
        v_layout.addWidget(self.__lineedit_agent_name)
        v_layout.addWidget(QLabel("提示词", self))
        v_layout.addWidget(self.__plaintext_edit_system_prompt)

    def _on_prompt_changed(self):
        self.settings_changed.emit({
            "agent_name": self.__lineedit_agent_name.text(),
            "system_prompt": self.__plaintext_edit_system_prompt.toPlainText()
        })

    def load_settings(self, agent_name: str, system_prompt: Optional[str]):
        self._loading_settings = True
        self.__lineedit_agent_name.setText(agent_name)
        self.__plaintext_edit_system_prompt.setPlainText(system_prompt or "")
        self._loading_settings = False


class PageModel(BaseWidget):
    __combobox_model: QComboBox
    __slider_model_temperature: QSlider
    __double_spinbox_model_temperature: QDoubleSpinBox
    __checkbox_max_tokens_number: QCheckBox
    __spinbox_max_tokens_number: QSpinBox
    __checkbox_streaming_output: QCheckBox
    __pushbutton_reset: QPushButton

    _loading_settings: bool = False
    settings_changed = Signal(dict)

    def __init__(self, parent: QWidget | None = None):
        super().__init__(parent)

    def _init_items(self):
        # __combobox_model
        self.__combobox_model = QComboBox(self)
        # TODO 模型列表从配置管理器获取(当前还未存储这个配置)
        self.__combobox_model.addItems([
            "deepseek:deepseek-chat"
        ])
        self.__combobox_model.currentTextChanged.connect(
            self.__emit_model_update_slot)  # Connect to new slot
        # __slider_model_temperature & __double_spinbox_model_temperature
        self.__slider_model_temperature = QSlider(self)
        self.__slider_model_temperature.setOrientation(
            Qt.Orientation.Horizontal)
        self.__slider_model_temperature.setRange(0, 200)
        self.__double_spinbox_model_temperature = QDoubleSpinBox(self)
        self.__double_spinbox_model_temperature.setRange(0, 2)
        self.__double_spinbox_model_temperature.setSingleStep(0.01)
        self.__slider_model_temperature.valueChanged.connect(
            self.__on_slider_model_temperature_value_changed)
        self.__double_spinbox_model_temperature.valueChanged.connect(
            self.__on_double_spinbox_model_temperature_value_changed)

        # __slider_model_top_p & __double_spinbox_model_top_p
        self.__slider_model_top_p = QSlider(self)
        self.__slider_model_top_p.setOrientation(Qt.Orientation.Horizontal)
        self.__slider_model_top_p.setRange(0, 100)
        self.__double_spinbox_model_top_p = QDoubleSpinBox(self)
        self.__double_spinbox_model_top_p.setRange(0, 1)
        self.__double_spinbox_model_top_p.setSingleStep(0.01)
        self.__slider_model_top_p.valueChanged.connect(
            self.__on_slider_model_top_p_value_changed)
        self.__double_spinbox_model_top_p.valueChanged.connect(
            self.__on_double_spinbox_model_top_p_value_changed)

        # __slider_model_contexts_number & __spinbox_model_contexts_number
        self.__slider_model_contexts_number = QSlider(self)
        self.__slider_model_contexts_number.setRange(0, 100)
        self.__slider_model_contexts_number.setOrientation(
            Qt.Orientation.Horizontal)
        self.__slider_model_contexts_number.setDisabled(True) # TODO 暂未实现
        self.__spinbox_model_contexts_number = QSpinBox(self)
        self.__spinbox_model_contexts_number.setRange(0, 100)
        self.__spinbox_model_contexts_number.setDisabled(True) # TODO 暂未实现
        self.__slider_model_contexts_number.valueChanged.connect(
            self.__on_slider_model_contexts_number_value_changed)
        self.__spinbox_model_contexts_number.valueChanged.connect(
            self.__on_spinbox_model_contexts_number_value_changed)

        # __checkbox_max_tokens_number & __spinbox_max_tokens_number
        self.__checkbox_max_tokens_number = QCheckBox(self)
        self.__checkbox_max_tokens_number.setText("最大Token数 (0表示无限制或模型默认)")
        self.__spinbox_max_tokens_number = QSpinBox(self)
        self.__spinbox_max_tokens_number.setRange(0, 10000000)
        self.__spinbox_max_tokens_number.hide()
        self.__checkbox_max_tokens_number.checkStateChanged.connect(
            self.__on_checkbox_max_tokens_number_check_state_changed)
        self.__spinbox_max_tokens_number.valueChanged.connect(
            self.__emit_model_update_slot)

        # __checkbox_streaming_output
        self.__checkbox_streaming_output = QCheckBox(self)
        self.__checkbox_streaming_output.setText("流式输出")
        self.__checkbox_streaming_output.setDisabled(True)  # TODO 暂未实现
        self.__checkbox_streaming_output.checkStateChanged.connect(
            self.__emit_model_update_slot)

        # __pushbutton_reset
        self.__pushbutton_reset = QPushButton(self)
        self.__pushbutton_reset.setText("重置为默认值")
        self.__pushbutton_reset.clicked.connect(
            self.__on_pushbutton_reset_clicked)

    def _init_layout(self):
        h_layout_model_temperature = QHBoxLayout()
        h_layout_model_temperature.addWidget(
            self.__slider_model_temperature, 8)
        h_layout_model_temperature.addWidget(
            self.__double_spinbox_model_temperature, 2)
        h_layout_model_top_p = QHBoxLayout()
        h_layout_model_top_p.addWidget(self.__slider_model_top_p, 8)
        h_layout_model_top_p.addWidget(self.__double_spinbox_model_top_p, 2)
        h_layout_model_contexts_number = QHBoxLayout()
        h_layout_model_contexts_number.addWidget(
            self.__slider_model_contexts_number, 8)
        h_layout_model_contexts_number.addWidget(
            self.__spinbox_model_contexts_number, 2)

        v_layout = QVBoxLayout(self)
        v_layout.addWidget(QLabel("模型选择", self))
        v_layout.addWidget(self.__combobox_model, 0,
                           Qt.AlignmentFlag.AlignLeft)
        v_layout.addWidget(QLabel("模型温度", self))
        v_layout.addLayout(h_layout_model_temperature)
        v_layout.addWidget(QLabel("Top-P", self))
        v_layout.addLayout(h_layout_model_top_p)
        v_layout.addWidget(QLabel("上下文数", self))
        v_layout.addLayout(h_layout_model_contexts_number)
        v_layout.addWidget(self.__checkbox_max_tokens_number)
        v_layout.addWidget(self.__spinbox_max_tokens_number)
        v_layout.addWidget(self.__checkbox_streaming_output)
        v_layout.addWidget(self.__pushbutton_reset, 0,
                           Qt.AlignmentFlag.AlignRight)
        v_layout.addStretch()

    def __emit_model_update_slot(self):
        self.__emit_model_update()

    def __emit_model_update(self):
        if self._loading_settings:
            return

        max_tokens_val = None
        if self.__checkbox_max_tokens_number.isChecked():
            max_tokens_val = self.__spinbox_max_tokens_number.value()
            if max_tokens_val == 0:
                max_tokens_val = None

        self.settings_changed.emit({
            "model": self.__combobox_model.currentText(),
            "temperature": self.__double_spinbox_model_temperature.value(),
            "top_p": self.__double_spinbox_model_top_p.value(),
            "context_size": self.__spinbox_model_contexts_number.value(),
            "max_tokens": max_tokens_val,
            "enable_streaming": self.__checkbox_streaming_output.isChecked()
        })

    def __on_slider_model_temperature_value_changed(self, value: int):
        if self._loading_settings:
            return
        self.__double_spinbox_model_temperature.setValue(value / 100.0)

    def __on_double_spinbox_model_temperature_value_changed(self, value: float):
        if self._loading_settings:
            return
        self.__slider_model_temperature.setValue(int(value * 100))
        self.__emit_model_update()

    def __on_slider_model_top_p_value_changed(self, value: int):
        if self._loading_settings:
            return
        self.__double_spinbox_model_top_p.setValue(value / 100.0)

    def __on_double_spinbox_model_top_p_value_changed(self, value: float):
        if self._loading_settings:
            return
        self.__slider_model_top_p.setValue(int(value * 100))
        self.__emit_model_update()

    def __on_slider_model_contexts_number_value_changed(self, value: int):
        if self._loading_settings:
            return
        self.__spinbox_model_contexts_number.setValue(value)

    def __on_spinbox_model_contexts_number_value_changed(self, value: int):
        if self._loading_settings:
            return
        self.__slider_model_contexts_number.setValue(value)
        self.__emit_model_update()

    def __on_checkbox_max_tokens_number_check_state_changed(self, state_value: int):
        state = Qt.CheckState(state_value)
        if self._loading_settings:
            return
        self.__spinbox_max_tokens_number.setVisible(
            state == Qt.CheckState.Checked)
        self.__emit_model_update()

    def __on_pushbutton_reset_clicked(self):
        if self._loading_settings:
            return
        EventBus().publish(EventBus.EventType.ButtonClicked, {
            "id": EventBus.Buttons.RESET_MODEL_ARGS,
            "message": "Request to reset model arguments to default"
        })

    def load_settings(self, config: AgentConfig):
        self._loading_settings = True
        cb_idx = self.__combobox_model.findText(config.model)
        self.__combobox_model.setCurrentIndex(cb_idx if cb_idx != -1 else 0)
        self.__double_spinbox_model_temperature.setValue(config.temperature)
        self.__slider_model_temperature.setValue(int(config.temperature * 100))
        self.__double_spinbox_model_top_p.setValue(config.top_p)
        self.__slider_model_top_p.setValue(int(config.top_p * 100))
        self.__spinbox_model_contexts_number.setValue(
            config.context_size or 0)  # Handle None
        self.__slider_model_contexts_number.setValue(config.context_size or 0)

        if config.max_tokens is not None and config.max_tokens > 0:
            self.__checkbox_max_tokens_number.setChecked(True)
            self.__spinbox_max_tokens_number.setValue(config.max_tokens)
            self.__spinbox_max_tokens_number.show()
        else:
            self.__checkbox_max_tokens_number.setChecked(False)
            self.__spinbox_max_tokens_number.setValue(0)
            self.__spinbox_max_tokens_number.hide()
        self.__checkbox_streaming_output.setChecked(config.enable_streaming)
        self._loading_settings = False


class PageMcpServer(BaseWidget):
    class WidgetMcpServer(BaseWidget):
        signal_check_state_changed = Signal(str, Qt.CheckState)
        __label_name: QLabel
        __label_description: QLabel
        __label_url: QLabel
        __checkbox: QCheckBox
        __id: str
        __name: str
        __description: str
        __url: str
        _loading_settings: bool = False

        def __init__(self, server_id: str, name: str, description: str, url: str, parent: QWidget | None = None):
            self.__id = server_id
            self.__name = name
            self.__description = description
            self.__url = url
            super().__init__(parent)

        def _init_widget(self): pass

        def _init_items(self):
            self.__label_name = QLabel(self.__name, self)
            self.__label_description = QLabel(self.__description, self)
            self.__label_url = QLabel(self.__url, self)
            self.__checkbox = QCheckBox(self)
            self.__checkbox.checkStateChanged.connect(
                self.__on_checkbox_check_state_changed)

        def _init_layout(self):
            v_layout_details = QVBoxLayout()
            v_layout_details.setContentsMargins(0, 0, 0, 0)
            v_layout_details.addWidget(self.__label_name)
            v_layout_details.addWidget(self.__label_description)
            v_layout_details.addWidget(self.__label_url)
            h_layout = QHBoxLayout(self)
            h_layout.addLayout(v_layout_details)
            h_layout.addWidget(self.__checkbox, 0, Qt.AlignmentFlag.AlignRight)

        def set_checked_silent(self, checked: bool):
            self._loading_settings = True
            self.__checkbox.setChecked(checked)
            self._loading_settings = False

        def __on_checkbox_check_state_changed(self, state_value: int):
            if self._loading_settings:
                return
            state = Qt.CheckState(state_value)
            self.signal_check_state_changed.emit(self.__id, state)

    __list_widget: QListWidget
    _loading_settings: bool = False
    settings_changed = Signal(dict)
    _current_mounted_ids: List[str]

    def __init__(self, parent: QWidget | None = None):
        self._current_mounted_ids = []
        super().__init__(parent)

    def _init_widget(self): pass

    def _init_items(self):
        self.__list_widget = QListWidget(self)

    def _init_layout(self):
        v_layout = QVBoxLayout(self)
        v_layout.addWidget(QLabel("MCP服务器挂载 (针对当前默认Agent模板)", self))
        v_layout.addWidget(self.__list_widget)

    def add_server_widget(self, server_id: str, name: str, description: str, url: str, is_mounted: bool):
        item = QListWidgetItem(self.__list_widget)
        widget_mcp_server = PageMcpServer.WidgetMcpServer(
            server_id, name, description, url, self.__list_widget
        )
        widget_mcp_server.signal_check_state_changed.connect(
            self.__on_item_mcp_selected_slot)
        item.setSizeHint(widget_mcp_server.sizeHint())
        self.__list_widget.addItem(item)
        self.__list_widget.setItemWidget(item, widget_mcp_server)
        widget_mcp_server.set_checked_silent(is_mounted)

    def __on_item_mcp_selected_slot(self, server_id: str, state: Qt.CheckState):
        self.__on_item_mcp_selected(server_id, state)

    def __on_item_mcp_selected(self, server_id: str, state: Qt.CheckState):
        if self._loading_settings:
            return

        if state == Qt.CheckState.Checked:
            if server_id not in self._current_mounted_ids:
                self._current_mounted_ids.append(server_id)
        else:
            if server_id in self._current_mounted_ids:
                self._current_mounted_ids.remove(server_id)

        self.settings_changed.emit(
            {"mounted_mcp_server_ids": list(self._current_mounted_ids)})

    def load_settings(self, all_mcp_servers: List[MCPServer], mounted_ids_for_template: List[str]):
        self._loading_settings = True
        self.__list_widget.clear()
        self._current_mounted_ids = list(mounted_ids_for_template)

        for server_config in all_mcp_servers:
            is_mounted = server_config.id in mounted_ids_for_template
            self.add_server_widget(
                server_config.id, server_config.name, server_config.description or "",
                server_config.url, is_mounted
            )
        self._loading_settings = False


class SettingsCentralWidget(BaseWidget):
    __stacked_layout: QStackedLayout
    __page_system_prompt: PageSystemPrompt
    __page_model: PageModel
    __page_mcp_server: PageMcpServer

    combined_settings_changed = Signal(dict)

    def __init__(self, parent: QWidget | None = None):
        super().__init__(parent)

    def _init_widget(self):
        pass

    def _init_items(self):
        self.__page_system_prompt = PageSystemPrompt(self)
        self.__page_model = PageModel(self)
        self.__page_mcp_server = PageMcpServer(self)
        self.__page_system_prompt.settings_changed.connect(
            self._on_child_page_settings_changed)
        self.__page_model.settings_changed.connect(
            self._on_child_page_settings_changed)
        self.__page_mcp_server.settings_changed.connect(
            self._on_child_page_settings_changed)

    def _init_layout(self):
        self.__stacked_layout = QStackedLayout(self)
        self.__stacked_layout.setContentsMargins(0, 0, 0, 0)
        self.__stacked_layout.addWidget(self.__page_system_prompt)
        self.__stacked_layout.addWidget(self.__page_model)
        self.__stacked_layout.addWidget(self.__page_mcp_server)

    def _on_child_page_settings_changed(self, changes: dict):
        self.combined_settings_changed.emit(changes)

    def set_current_index(self, index: int):
        if 0 <= index < self.__stacked_layout.count():
            self.__stacked_layout.setCurrentIndex(index)

    def load_all_page_settings(self, config: AgentConfig, all_mcp_servers: List[MCPServer]):
        self.__page_system_prompt.load_settings(ConfigManager(
        ).get_current_app_config().default_config_name, config.system_prompt)
        self.__page_model.load_settings(config)
        self.__page_mcp_server.load_settings(
            all_mcp_servers, config.mounted_mcp_server_ids)


class DialogSettings(QDialog):
    __splitter: QSplitter
    __list_widget: QListWidget
    __settings_central_widget: SettingsCentralWidget
    __button_box: QDialogButtonBox
    _temp_settings: TempAgentSettings

    def __init__(self, parent: QWidget | None = None):
        super().__init__(parent)
        self._temp_settings = TempAgentSettings()
        self._init_widget()
        self._init_items()
        self._init_layout()
        EventBus().signal_config_event.connect(self._handle_config_changed_externally)

    def _init_widget(self):
        self.setWindowTitle("设置 (编辑默认Agent模板)")
        self.setModal(True)
        self.setMinimumSize(700, 500)

    def _init_items(self):
        self.__list_widget = QListWidget(self)
        self.__list_widget.addItem("提示词设置")
        self.__list_widget.addItem("模型设置")
        self.__list_widget.addItem("MCP服务器挂载")
        self.__list_widget.currentItemChanged.connect(
            self.__on_selection_changed)

        self.__settings_central_widget = SettingsCentralWidget(self)
        self.__settings_central_widget.combined_settings_changed.connect(
            self._update_temp_settings)

        self.__splitter = QSplitter(self)
        self.__splitter.setChildrenCollapsible(False)
        self.__splitter.addWidget(self.__list_widget)
        self.__splitter.addWidget(self.__settings_central_widget)
        self.__splitter.setStretchFactor(0, 2)
        self.__splitter.setStretchFactor(1, 8)

        self.__button_box = QDialogButtonBox(
            QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel, self)
        self.__button_box.accepted.connect(self.accept)
        self.__button_box.rejected.connect(self.reject)

    def _init_layout(self):
        main_layout = QVBoxLayout(self)
        main_layout.addWidget(self.__splitter)
        main_layout.addWidget(self.__button_box)

    def _update_temp_settings(self, changes: dict):
        logger.debug(
            f"DialogSettings received changes from child page: {changes}")
        self._temp_settings.update_from_dict(changes)

    def _handle_config_changed_externally(self, event_payload: dict):
        if self.isVisible():
            event_id = event_payload.get("id")
            data = event_payload.get("data")
            config_manager = ConfigManager()
            current_default_name = config_manager.get_current_app_config().default_config_name

            reload_needed = False
            if event_id == EventBus.ConfigEvents.AGENT_TEMPLATE_UPDATED:
                if data.get("template_name") == current_default_name:
                    reload_needed = True
            elif event_id == EventBus.ConfigEvents.DEFAULT_AGENT_TEMPLATE_CHANGED:
                reload_needed = True  # Default template itself changed name or content

            if reload_needed:
                logger.info(
                    "DialogSettings detected external change to default template, reloading UI.")
                self.__load_settings_into_ui()

    def __load_settings_into_ui(self):
        logger.debug(
            "DialogSettings: Loading current settings into UI elements...")
        config_manager = ConfigManager()
        app_config = config_manager.get_current_app_config()
        default_template_name = app_config.default_config_name
        default_template_config = config_manager.get_template(
            default_template_name)

        if not default_template_config:
            logger.error(
                f"Default template '{default_template_name}' not found during UI load. Creating a new default.")
            default_template_config = config_manager.get_default_template()
            default_template_name = config_manager.get_current_app_config(
            ).default_config_name  # Get potentially new name

        self._temp_settings = TempAgentSettings()
        self._temp_settings.agent_name = default_template_name
        self._temp_settings.system_prompt = default_template_config.system_prompt
        self._temp_settings.model = default_template_config.model
        self._temp_settings.temperature = default_template_config.temperature
        self._temp_settings.top_p = default_template_config.top_p
        self._temp_settings.context_size = default_template_config.context_size
        self._temp_settings.max_tokens = default_template_config.max_tokens
        self._temp_settings.enable_streaming = default_template_config.enable_streaming
        self._temp_settings.mounted_mcp_server_ids = list(
            default_template_config.mounted_mcp_server_ids)

        all_mcp_s = config_manager.get_all_mcp_servers()
        self.__settings_central_widget.load_all_page_settings(
            default_template_config, all_mcp_s)
        logger.debug("DialogSettings: UI elements populated.")

    def showEvent(self, event):
        logger.info("Settings dialog showEvent: loading current configuration.")
        self.__load_settings_into_ui()
        if self.__list_widget.currentRow() == -1 and self.__list_widget.count() > 0:
            self.__list_widget.setCurrentRow(0)
        else:
            self.__on_selection_changed()
        super().showEvent(event)

    def accept(self):
        logger.info("DialogSettings accepted. Applying changes.")
        config_manager = ConfigManager()
        original_default_config = config_manager.get_default_template()
        changes_to_apply = {}
        if self._temp_settings.system_prompt is not None and \
           self._temp_settings.system_prompt != original_default_config.system_prompt:
            changes_to_apply['system_prompt'] = self._temp_settings.system_prompt
        if self._temp_settings.model is not None and self._temp_settings.model != original_default_config.model:
            changes_to_apply['model'] = self._temp_settings.model
        if self._temp_settings.temperature is not None and abs(self._temp_settings.temperature - original_default_config.temperature) > 1e-9:
            changes_to_apply['temperature'] = self._temp_settings.temperature
        if self._temp_settings.top_p is not None and abs(self._temp_settings.top_p - original_default_config.top_p) > 1e-9:
            changes_to_apply['top_p'] = self._temp_settings.top_p
        if self._temp_settings.context_size is not None and self._temp_settings.context_size != original_default_config.context_size:
            changes_to_apply['context_size'] = self._temp_settings.context_size
        if self._temp_settings.max_tokens != original_default_config.max_tokens:
            changes_to_apply['max_tokens'] = self._temp_settings.max_tokens
        if self._temp_settings.enable_streaming is not None and self._temp_settings.enable_streaming != original_default_config.enable_streaming:
            changes_to_apply['enable_streaming'] = self._temp_settings.enable_streaming
        if self._temp_settings.mounted_mcp_server_ids is not None and \
           set(self._temp_settings.mounted_mcp_server_ids) != set(original_default_config.mounted_mcp_server_ids):
            changes_to_apply['mounted_mcp_server_ids'] = self._temp_settings.mounted_mcp_server_ids
        if changes_to_apply:
            logger.info(
                f"Applying changes to default template '{original_default_config.default_config_name if hasattr(original_default_config, 'default_config_name') else ConfigManager().get_current_app_config().default_config_name}': {changes_to_apply}")
            default_template_name = ConfigManager().get_current_app_config().default_config_name
            config_manager.update_template(
                default_template_name, **changes_to_apply)
        else:
            logger.info("No changes detected to apply.")
        super().accept()

    def reject(self):
        logger.info("DialogSettings rejected. No changes applied.")
        super().reject()

    def __on_selection_changed(self):
        self.__settings_central_widget.set_current_index(
            self.__list_widget.currentIndex().row())
