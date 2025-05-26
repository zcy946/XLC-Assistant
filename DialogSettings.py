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
from ConfigManager import ConfigManager, AgentConfig, MCPServer # Import full ConfigManager and models


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
        self.__lineedit_agent_name.setPlaceholderText("智能体名称 (当前为默认模板)")
        self.__lineedit_agent_name.setReadOnly(True) # Edits default template for now
        # __plaintext_edit_system_prompt
        self.__plaintext_edit_system_prompt = QPlainTextEdit(self)
        self.__plaintext_edit_system_prompt.setPlaceholderText("智能体提示词")
        # __pushbutton_save
        self.__pushbutton_save = QPushButton(self)
        self.__pushbutton_save.setText("保存提示词") # More specific text
        self.__pushbutton_save.clicked.connect(
            self.__on_pushbutton_save_clicked)

    def _init_layout(self):
        v_layout = QVBoxLayout(self)
        v_layout.addWidget(QLabel("当前编辑模板名称", self)) # Clarified label
        v_layout.addWidget(self.__lineedit_agent_name)
        v_layout.addWidget(QLabel("提示词", self))
        v_layout.addWidget(self.__plaintext_edit_system_prompt)
        v_layout.addWidget(self.__pushbutton_save, 0,
                           Qt.AlignmentFlag.AlignRight)

    def __on_pushbutton_save_clicked(self):
        # agent_name is read from the line edit, which should be populated with the default template name
        EventBus().publish(EventBus.EventType.ButtonClicked, {
            "id": EventBus.Buttons.UPDATE_SYSTEM_PROMPT,
            "message": "Update system prompt for default agent", # Clarified message
            "data": { # This is the payload ConfigManager expects under "data" from ButtonClicked event
                "agent": {
                    "agent_name": self.__lineedit_agent_name.text(), # This will be the default template name
                    "system_prompt": self.__plaintext_edit_system_prompt.toPlainText()
                }
            }
        })
        logger.info(f"Save system prompt button clicked for agent: {self.__lineedit_agent_name.text()}")


class PageModel(BaseWidget):
    __combobox_model: QComboBox
    __slider_model_temperature: QSlider
    __double_spinbox_model_temperature: QDoubleSpinBox
    __slider_model_top_p: QSlider
    __double_spinbox_model_top_p: QDoubleSpinBox
    __slider_model_contexts_number: QSlider # UI for "上下文数"
    __spinbox_model_contexts_number: QSpinBox # UI for "上下文数"
    __checkbox_max_tokens_number: QCheckBox
    __spinbox_max_tokens_number: QSpinBox
    __checkbox_streaming_output: QCheckBox
    __pushbutton_reset: QPushButton
    
    # Flag to prevent __update_model during programmatic changes by __load_settings
    _loading_settings: bool = False


    def __init__(self, parent: QWidget | None = None):
        super().__init__(parent)

    def _init_widget(self):
        pass

    def _init_items(self):
        # __combobox_model
        self.__combobox_model = QComboBox(self)
        # TODO: Populate this dynamically or ensure a comprehensive static list
        self.__combobox_model.addItems([
            "deepseek:deepseek-chat", 
            "deepseek:deepseek-coder", # Example
            "openai:gpt-4o", # Example
            "openai:gpt-3.5-turbo" # Example
        ])
        self.__combobox_model.currentTextChanged.connect(self.__trigger_model_update)
        # __slider_model_temperature
        self.__slider_model_temperature = QSlider(self)
        self.__slider_model_temperature.setOrientation(
            Qt.Orientation.Horizontal)
        self.__slider_model_temperature.setRange(0, 200)
        self.__slider_model_temperature.valueChanged.connect(
            self.__on_slider_model_temperature_value_changed)
        # __double_spinbox_model_temperature
        self.__double_spinbox_model_temperature = QDoubleSpinBox(self)
        self.__double_spinbox_model_temperature.setRange(0, 2)
        self.__double_spinbox_model_temperature.setSingleStep(0.01)
        self.__double_spinbox_model_temperature.valueChanged.connect(
            self.__on_double_spinbox_model_temperature_value_changed)
        # __slider_model_top_p
        self.__slider_model_top_p = QSlider(self)
        self.__slider_model_top_p.setOrientation(Qt.Orientation.Horizontal)
        self.__slider_model_top_p.setRange(0, 100)
        self.__slider_model_top_p.valueChanged.connect(
            self.__on_slider_model_top_p_value_changed)
        # __double_spinbox_model_top_p
        self.__double_spinbox_model_top_p = QDoubleSpinBox(self)
        self.__double_spinbox_model_top_p.setRange(0, 1)
        self.__double_spinbox_model_top_p.setSingleStep(0.01)
        self.__double_spinbox_model_top_p.valueChanged.connect(
            self.__on_double_spinbox_model_top_p_value_changed)
        # __slider_contexts_number (UI's "上下文数" - maps to AgentConfig.context_size)
        self.__slider_model_contexts_number = QSlider(self)
        self.__slider_model_contexts_number.setRange(0, 8192) # Example range, adjust as needed for typical context_size
        self.__slider_model_contexts_number.setOrientation(
            Qt.Orientation.Horizontal)
        self.__slider_model_contexts_number.valueChanged.connect(
            self.__on_slider_model_contexts_number_value_changed)
        # __spinbox_model_context_number (UI's "上下文数")
        self.__spinbox_model_contexts_number = QSpinBox(self)
        self.__spinbox_model_contexts_number.setRange(0, 8192) # Example range
        self.__spinbox_model_contexts_number.valueChanged.connect(
            self.__on_spinbox_model_contexts_number_value_changed)
        # __checkbox_max_tokens_number
        self.__checkbox_max_tokens_number = QCheckBox(self)
        self.__checkbox_max_tokens_number.setText("最大Token数 (0表示无限制或模型默认)")
        self.__checkbox_max_tokens_number.checkStateChanged.connect(
            self.__on_checkbox_max_tokens_number_check_state_changed)
        # __spinbox_max_tokens_number
        self.__spinbox_max_tokens_number = QSpinBox(self)
        self.__spinbox_max_tokens_number.setRange(0, 10000000) # 0 can mean None/default
        self.__spinbox_max_tokens_number.hide()
        self.__spinbox_max_tokens_number.valueChanged.connect(self.__trigger_model_update)
        # __checkbox_streaming_output
        self.__checkbox_streaming_output = QCheckBox(self)
        self.__checkbox_streaming_output.setText("流式输出")
        self.__checkbox_streaming_output.checkStateChanged.connect(self.__trigger_model_update)
        # __pushbutton_reset
        self.__pushbutton_reset = QPushButton(self)
        self.__pushbutton_reset.setText("重置为默认值")
        self.__pushbutton_reset.clicked.connect(
            self.__on_pushbutton_reset_clicked)

    def _init_layout(self):
        h_layout_model_temperature = QHBoxLayout()
        h_layout_model_temperature.addWidget(self.__slider_model_temperature, 8)
        h_layout_model_temperature.addWidget(self.__double_spinbox_model_temperature, 2)
        h_layout_model_top_p = QHBoxLayout()
        h_layout_model_top_p.addWidget(self.__slider_model_top_p, 8)
        h_layout_model_top_p.addWidget(self.__double_spinbox_model_top_p, 2)
        h_layout_model_contexts_number = QHBoxLayout()
        h_layout_model_contexts_number.addWidget(self.__slider_model_contexts_number, 8)
        h_layout_model_contexts_number.addWidget(self.__spinbox_model_contexts_number, 2)
        
        v_layout = QVBoxLayout(self)
        v_layout.addWidget(QLabel("模型选择", self))
        v_layout.addWidget(self.__combobox_model, 0, Qt.AlignmentFlag.AlignLeft)
        v_layout.addWidget(QLabel("模型温度 (Temperature)", self))
        v_layout.addLayout(h_layout_model_temperature)
        v_layout.addWidget(QLabel("Top-P", self))
        v_layout.addLayout(h_layout_model_top_p)
        v_layout.addWidget(QLabel("上下文Token数 (Context Size / context_length)", self)) # Clarified label
        v_layout.addLayout(h_layout_model_contexts_number)
        v_layout.addWidget(self.__checkbox_max_tokens_number)
        v_layout.addWidget(self.__spinbox_max_tokens_number)
        v_layout.addWidget(self.__checkbox_streaming_output)
        v_layout.addWidget(self.__pushbutton_reset, 0, Qt.AlignmentFlag.AlignRight)
        v_layout.addStretch()

    def __trigger_model_update(self): # Renamed from __update_model to avoid confusion
        if self._loading_settings: # Don't emit events while loading
            return
        
        _model = self.__combobox_model.currentText()
        _temperature = self.__double_spinbox_model_temperature.value()
        _top_p = self.__double_spinbox_model_top_p.value()
        _contexts_number = self.__spinbox_model_contexts_number.value() # This is UI's "上下文数"
        
        if self.__checkbox_max_tokens_number.isChecked():
            _max_tokens = self.__spinbox_max_tokens_number.value()
            if _max_tokens == 0: # Treat 0 as None (model default or no limit)
                _max_tokens = None
        else:
            _max_tokens = None 
            
        _streaming_output = self.__checkbox_streaming_output.isChecked()

        EventBus().publish(EventBus.EventType.StateChanged, {
            "id": EventBus.States.MODEL_UPDATED,
            "message": "Model parameters updated by user",
            "data": {
                "model": _model,
                "temperature": _temperature,
                "top_p": _top_p,
                "contexts_number": _contexts_number, # Will be mapped to context_size by ConfigManager
                "max_tokens": _max_tokens,
                "streaming_output": _streaming_output
            }
        })
        logger.debug(f"PageModel sent MODEL_UPDATED event with data: model={_model}, temp={_temperature}, top_p={_top_p}, context_size(UI)={_contexts_number}, max_tokens={_max_tokens}, stream={_streaming_output}")


    def __on_slider_model_temperature_value_changed(self, value: int):
        if self._loading_settings: return
        self.__double_spinbox_model_temperature.setValue(value / 100.0)
        # __double_spinbox will trigger __trigger_model_update if its valueChanged connects directly to it

    def __on_double_spinbox_model_temperature_value_changed(self, value: float):
        if self._loading_settings: return
        self.__slider_model_temperature.setValue(int(value * 100))
        self.__trigger_model_update()


    def __on_slider_model_top_p_value_changed(self, value: int):
        if self._loading_settings: return
        self.__double_spinbox_model_top_p.setValue(value / 100.0)

    def __on_double_spinbox_model_top_p_value_changed(self, value: float):
        if self._loading_settings: return
        self.__slider_model_top_p.setValue(int(value * 100))
        self.__trigger_model_update()

    def __on_slider_model_contexts_number_value_changed(self, value: int):
        if self._loading_settings: return
        self.__spinbox_model_contexts_number.setValue(value)
        # __spinbox will trigger __trigger_model_update

    def __on_spinbox_model_contexts_number_value_changed(self, value: int):
        if self._loading_settings: return
        self.__slider_model_contexts_number.setValue(value)
        self.__trigger_model_update()

    def __on_checkbox_max_tokens_number_check_state_changed(self, state_value: int): # state is int
        state = Qt.CheckState(state_value)
        if self._loading_settings: return
        self.__spinbox_max_tokens_number.setVisible(state == Qt.CheckState.Checked)
        self.__trigger_model_update()

    def __on_pushbutton_reset_clicked(self):
        if self._loading_settings: return
        logger.debug("PageModel: Reset button clicked, emitting RESET_MODEL_ARGS")
        EventBus().publish(EventBus.EventType.ButtonClicked, {
            "id": EventBus.Buttons.RESET_MODEL_ARGS,
            "message": "Request to reset model arguments to default"
            # No data needed here, ConfigManager knows the defaults for the current template
        })
        # The UI will be updated by __load_settings after ConfigManager processes the reset and reloads.


class PageMcpServer(BaseWidget):
    class WidgetMcpServer(BaseWidget):
        signal_check_state_changed = Signal(str, Qt.CheckState)
        __label_name: QLabel
        __label_description: QLabel
        __label_url: QLabel
        __checkbox: QCheckBox
        __id: str # Store server ID
        __name: str
        __description: str
        __url: str
        _loading_settings: bool = False # To prevent signals during programmatic check changes

        def __init__(self, server_id: str, name: str, description: str, url: str, parent: QWidget | None = None):
            self.__id = server_id # Store the ID
            self.__name = name
            self.__description = description
            self.__url = url
            super().__init__(parent)

        def _init_widget(self):
            pass

        def _init_items(self):
            self.__label_name = QLabel(self.__name, self)
            self.__label_description = QLabel(self.__description, self)
            self.__label_url = QLabel(self.__url, self)
            self.__checkbox = QCheckBox(self)
            self.__checkbox.checkStateChanged.connect(self.__on_checkbox_check_state_changed)

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

        def __on_checkbox_check_state_changed(self, state_value: int): # state is int
            if self._loading_settings:
                return
            state = Qt.CheckState(state_value)
            self.signal_check_state_changed.emit(self.__id, state) # Emit server_id

    __list_widget: QListWidget
    # __label_state_selected: QLabel # This was unused, remove
    # __mcp_servers: list[dict[str, str]] # This seems like local cache, might not be needed if always reading from ConfigManager
    __selected_mcp_servers_cache: list[dict[str, str]] # Cache for current selections based on UI interactions
    _loading_settings: bool = False # To prevent signals during programmatic changes

    def __init__(self, parent: QWidget | None = None):
        # self.__mcp_servers = [] # Removed, will load dynamically
        self.__selected_mcp_servers_cache = []
        super().__init__(parent)
        # TEST data removed, will be populated by __load_settings

    def _init_widget(self):
        pass

    def _init_items(self):
        self.__list_widget = QListWidget(self)

    def _init_layout(self):
        v_layout = QVBoxLayout(self)
        v_layout.addWidget(QLabel("MCP服务器挂载 (针对当前默认Agent模板)", self))
        v_layout.addWidget(self.__list_widget)

    def add_server_widget(self, server_id: str, name: str, description: str, url: str, is_mounted: bool):
        """Helper to add a server widget to the list and set its mounted state."""
        item = QListWidgetItem(self.__list_widget)
        widget_mcp_server = PageMcpServer.WidgetMcpServer(
            server_id, name, description, url, self.__list_widget
        )
        widget_mcp_server.signal_check_state_changed.connect(self.__on_item_mcp_selected)
        item.setSizeHint(widget_mcp_server.sizeHint())
        self.__list_widget.addItem(item)
        self.__list_widget.setItemWidget(item, widget_mcp_server)
        
        widget_mcp_server.set_checked_silent(is_mounted) # Set initial state without emitting signal
        if is_mounted and not any(s['id'] == server_id for s in self.__selected_mcp_servers_cache):
            self.__selected_mcp_servers_cache.append({"id": server_id})


    def __on_item_mcp_selected(self, server_id: str, state: Qt.CheckState):
        if self._loading_settings:
            return

        logger.debug(f"MCP server '{server_id}' selection changed to: {state}")
        
        # Update internal cache
        is_selected = any(s["id"] == server_id for s in self.__selected_mcp_servers_cache)
        if state == Qt.CheckState.Checked and not is_selected:
            self.__selected_mcp_servers_cache.append({"id": server_id})
        elif state == Qt.CheckState.Unchecked and is_selected:
            self.__selected_mcp_servers_cache = [s for s in self.__selected_mcp_servers_cache if s["id"] != server_id]

        EventBus().publish(EventBus.EventType.StateChanged, {
            "id": EventBus.States.MCP_SERVERS_UPDATED,
            "message": "MCP Servers mount status updated for default agent",
            "data": {
                "mcp_servers": list(self.__selected_mcp_servers_cache) # Send a copy
            }
        })
        logger.debug(f"PageMcpServer sent MCP_SERVERS_UPDATED event with data: {self.__selected_mcp_servers_cache}")


class SettingsCentralWidget(BaseWidget):
    __stacked_layout: QStackedLayout
    __page_system_prompt: PageSystemPrompt
    __page_model: PageModel
    __page_mcp_server: PageMcpServer

    def __init__(self, parent: QWidget | None = None):
        super().__init__(parent)

    def _init_widget(self): pass
    def _init_items(self):
        self.__page_system_prompt = PageSystemPrompt(self)
        self.__page_model = PageModel(self)
        self.__page_mcp_server = PageMcpServer(self)
    def _init_layout(self):
        self.__stacked_layout = QStackedLayout(self)
        self.__stacked_layout.setContentsMargins(0, 0, 0, 0)
        self.__stacked_layout.addWidget(self.__page_system_prompt)
        self.__stacked_layout.addWidget(self.__page_model)
        self.__stacked_layout.addWidget(self.__page_mcp_server)

    def set_current_index(self, index: int):
        if 0 <= index < self.__stacked_layout.count():
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
        # ConfigManager might emit AGENT_TEMPLATE_UPDATED if a reset button causes a save.
        # If this dialog is open, it should refresh its view.
        EventBus().signal_config_event.connect(self._handle_config_changed_externally)


    def _init_widget(self):
        self.setWindowTitle("设置 (编辑默认Agent模板)")
        self.setModal(True)

    def _init_items(self):
        self.__list_widget = QListWidget(self)
        self.__list_widget.addItem("提示词设置")
        self.__list_widget.addItem("模型设置")
        self.__list_widget.addItem("MCP服务器挂载")
        self.__list_widget.currentItemChanged.connect(self.__on_selection_changed)
        self.__settings_central_widget = SettingsCentralWidget(self)
        self.__splitter = QSplitter(self)
        self.__splitter.setChildrenCollapsible(False)
        self.__splitter.addWidget(self.__list_widget)
        self.__splitter.addWidget(self.__settings_central_widget)
        self.__splitter.setStretchFactor(0, 2)
        self.__splitter.setStretchFactor(1, 8)

    def _init_layout(self):
        v_layout = QVBoxLayout(self)
        # v_layout.addSpacing(0) # Not strictly needed
        v_layout.addWidget(self.__splitter)

    def _handle_config_changed_externally(self, event_payload: dict):
        # If the dialog is visible and the config it's based on (default template) changes, reload.
        event_id = event_payload.get("id")
        data = event_payload.get("data")
        config_manager = ConfigManager()
        
        if self.isVisible(): # This is key
            is_default_template_event = False
            if event_id == EventBus.ConfigEvents.AGENT_TEMPLATE_UPDATED:
                updated_template_name = data.get("template_name")
                if updated_template_name == config_manager.get_current_app_config().default_config_name:
                    is_default_template_event = True
            elif event_id == EventBus.ConfigEvents.DEFAULT_AGENT_TEMPLATE_CHANGED: # This event is also relevant
                is_default_template_event = True # If the default template *itself* changes, we also need to reload
            
            if is_default_template_event:
                logger.info("DialogSettings detected external change to default template, reloading UI.")
                self.__load_settings() # This should refresh the UI


    def __load_settings(self):
        logger.debug("DialogSettings: Loading settings into UI elements...")
        config_manager = ConfigManager()
        default_template_config = config_manager.get_default_template() # This handles fallbacks gracefully
        default_template_name = config_manager.get_current_app_config().default_config_name

        # --- Populate PageSystemPrompt ---
        page_sys_prompt = self.__settings_central_widget._SettingsCentralWidget__page_system_prompt
        page_sys_prompt._PageSystemPrompt__lineedit_agent_name.setText(default_template_name)
        page_sys_prompt._PageSystemPrompt__plaintext_edit_system_prompt.setPlainText(default_template_config.system_prompt or "")

        # --- Populate PageModel ---
        page_model = self.__settings_central_widget._SettingsCentralWidget__page_model
        page_model._loading_settings = True # Prevent event emissions

        # Model ComboBox
        cb_idx = page_model._PageModel__combobox_model.findText(default_template_config.model)
        if cb_idx != -1:
            page_model._PageModel__combobox_model.setCurrentIndex(cb_idx)
        else:
            logger.warning(f"Model '{default_template_config.model}' not in PageModel combobox. Adding it or defaulting.")
            # Optionally add it: page_model._PageModel__combobox_model.addItem(default_template_config.model)
            page_model._PageModel__combobox_model.setCurrentIndex(0) # Fallback to first item

        page_model._PageModel__double_spinbox_model_temperature.setValue(default_template_config.temperature)
        page_model._PageModel__double_spinbox_model_top_p.setValue(default_template_config.top_p)
        
        # Context Size (UI's "上下文数")
        # AgentConfig.context_size is Optional[int]. Spinbox has a range.
        page_model._PageModel__spinbox_model_contexts_number.setValue(default_template_config.context_size or 0) # Default to 0 if None

        # Max Tokens
        if default_template_config.max_tokens is not None and default_template_config.max_tokens > 0:
            page_model._PageModel__checkbox_max_tokens_number.setChecked(True)
            page_model._PageModel__spinbox_max_tokens_number.setValue(default_template_config.max_tokens)
            page_model._PageModel__spinbox_max_tokens_number.show()
        else:
            page_model._PageModel__checkbox_max_tokens_number.setChecked(False)
            page_model._PageModel__spinbox_max_tokens_number.setValue(0) # Default spinbox value
            page_model._PageModel__spinbox_max_tokens_number.hide()
            
        page_model._PageModel__checkbox_streaming_output.setChecked(default_template_config.enable_streaming)
        
        page_model._loading_settings = False # Done loading PageModel

        # --- Populate PageMcpServer ---
        page_mcp = self.__settings_central_widget._SettingsCentralWidget__page_mcp_server
        page_mcp._loading_settings = True # Prevent event emissions
        page_mcp._PageMcpServer__list_widget.clear()
        page_mcp._PageMcpServer__selected_mcp_servers_cache.clear()

        all_mcp_servers = config_manager.get_all_mcp_servers()
        mounted_ids = set(default_template_config.mounted_mcp_server_ids)

        for server_config in all_mcp_servers:
            is_mounted = server_config.id in mounted_ids
            page_mcp.add_server_widget(
                server_config.id, server_config.name, server_config.description or "", server_config.url, is_mounted
            )
        
        page_mcp._loading_settings = False # Done loading PageMcpServer
        logger.debug("DialogSettings: UI elements populated with current default config.")


    def showEvent(self, event):
        logger.info("Settings dialog showEvent: loading current configuration.")
        self.__load_settings()
        # Set initial page selection (e.g., first item)
        if self.__list_widget.currentRow() == -1 and self.__list_widget.count() > 0:
            self.__list_widget.setCurrentRow(0)
        else: # Ensure the central widget updates if a row is already selected
            self.__on_selection_changed()

        super().showEvent(event)

    def closeEvent(self, event):
        logger.debug("Settings dialog closed/hid. Changes are saved on interaction.")
        super().closeEvent(event)

    def __on_selection_changed(self):
        self.__settings_central_widget.set_current_index(
            self.__list_widget.currentIndex().row())