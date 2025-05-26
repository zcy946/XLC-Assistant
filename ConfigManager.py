from pydantic import BaseModel, Field, ValidationError
from typing import Optional, Dict, Any, Literal, List
from loguru import logger
import json
import os
# from typing import Dict, Optional, List # Redundant
# from pydantic import ValidationError # Redundant
# from loguru import logger # Redundant
from Singleton import Singleton 
from EventBus import EventBus # 确保导入 EventBus

# --- MCP 服务器模型 ---
class MCPServer(BaseModel):
    id: str = Field(..., description="MCP 服务器的唯一标识符（例如：URL 或自定义 ID）。")
    name: str = Field(..., description="MCP 服务器的显示名称。")
    url: str = Field(..., description="MCP 服务器 API 的基础 URL。")
    api_key: Optional[str] = Field(None, description="用于 MCP 服务器认证的 API 密钥。")
    description: Optional[str] = Field(None, description="MCP 服务器的可选描述。")

# --- Agent 配置模型 ---
class AgentConfig(BaseModel):
    """
    PydanticAI Agent 的单个配置模板
    """
    model: str = Field(..., description="要使用的 AI 模型名称（例如：'gpt-4o'，'gpt-3.5-turbo'）。")
    temperature: float = Field(
        0.7, ge=0.0, le=2.0, description="控制生成的随机性。值越高，生成内容越随机。")
    top_p: float = Field(
        1.0, ge=0.0, le=1.0, description="通过 nucleus sampling 控制多样性。值越高，多样性越大。")
    max_tokens: Optional[int] = Field(
        None, gt=0, description="生成补全内容的最大 token 数。")
    system_prompt: Optional[str] = Field(
        None, description="用于引导 agent 行为的初始提示。")
    context_size: Optional[int] = Field(
        None, ge=0, description="上下文数量，越大，模型能记住的历史越多。")

    model_kwargs: Dict[str, Any] = Field(
        {}, description="传递给底层 AI 模型的其他关键字参数。")

    enable_streaming: bool = Field(
        False, description="是否为 agent 启用流式输出。")

    mounted_mcp_server_ids: List[str] = Field(
        default_factory=list,
        description="此 Agent 模板要挂载的 MCP 服务器的 ID 列表。"
    )

# --- 应用程序整体配置模型 ---
class AppConfig(BaseModel):
    """
    应用程序的整体配置，包含多个 AgentConfig 模板和 MCP 服务器列表
    """
    default_config_name: str = Field(
        "default", description="默认 agent 配置的名称。")
    agent_templates: Dict[str, AgentConfig] = Field(
        ..., description="命名的 agent 配置模板字典。")
    
    mcp_servers: List[MCPServer] = Field(
        default_factory=list,
        description="所有已配置的 MCP 服务器列表。"
    )

    def model_post_init(self, __context: Any) -> None:
        existing_mcp_ids = {server.id for server in self.mcp_servers}
        for template_name, agent_template in self.agent_templates.items():
            # 创建列表副本进行迭代，以安全修改原始列表
            ids_to_remove = []
            for server_id in agent_template.mounted_mcp_server_ids:
                if server_id not in existing_mcp_ids:
                    logger.warning(
                        f"Agent template '{template_name}' references unknown MCP server ID '{server_id}'. "
                        f"Removing '{server_id}' from template '{template_name}'."
                    )
                    ids_to_remove.append(server_id)
            
            for server_id in ids_to_remove:
                 agent_template.mounted_mcp_server_ids.remove(server_id)

        if len(existing_mcp_ids) != len(self.mcp_servers):
            logger.warning("Duplicate MCP server IDs found in mcp_servers. Only unique IDs will be considered.")


# --- 默认配置 ---
DEFAULT_MCP_SERVERS = [
    MCPServer(
        id="test_local_mcp",
        name="本地开发测试MCP",
        url="http://127.0.0.1:8000/sse",
        description="用于本地开发测试的MCP服务器。"
    )
]

DEFAULT_AGENT_CONFIGS = {
    "default": AgentConfig(
        model="deepseek:deepseek-chat", 
        temperature=0.7,
        top_p=1.0,
        max_tokens=None,
        system_prompt="你是一个有用的助手，帮助用户完成任务。",
        context_size=None, 
        enable_streaming=False, # 确保默认值存在
        mounted_mcp_server_ids=[]
    )
}

DEFAULT_APP_CONFIG = AppConfig(
    default_config_name="default",
    agent_templates=DEFAULT_AGENT_CONFIGS,
    mcp_servers=DEFAULT_MCP_SERVERS
)

class ConfigManager(Singleton):
    _config_file_path: str = os.path.join(
        os.path.dirname(__file__), "config.json")
    _current_config: AppConfig

    def __init__(self, config_file_path: Optional[str] = None):
        # Singleton pattern: prevent re-initialization
        if hasattr(self, '_initialized') and self._initialized:
            return
        
        if config_file_path:
            self._config_file_path = config_file_path
        
        self._load_config()
        self._subscribe_to_events() # 订阅来自UI的事件
        self._initialized = True # Mark as initialized

    def _subscribe_to_events(self):
        event_bus = EventBus()
        event_bus.signal_button_clicked.connect(self._handle_button_clicked)
        event_bus.signal_state_changed.connect(self._handle_state_changed)
        logger.info("ConfigManager subscribed to UI events.")

    def _handle_button_clicked(self, event_data: dict):
        event_id = event_data.get("id")
        data = event_data.get("data") # 正确获取 data 字典

        if event_id == EventBus.Buttons.UPDATE_SYSTEM_PROMPT:
            logger.debug(f"ConfigManager handling UPDATE_SYSTEM_PROMPT: {data}")
            # DialogSettings 发送的结构是: {"id": ..., "message": ..., "data": {"agent": ...}}
            agent_payload = data.get("agent", {}) 

            target_template_name = agent_payload.get("agent_name")
            new_system_prompt = agent_payload.get("system_prompt")

            if not target_template_name: # 如果UI没有提供名称，则默认为当前应用的默认模板
                logger.warning("No agent_name provided for system prompt update. Using current default.")
                target_template_name = self._current_config.default_config_name
            
            if new_system_prompt is not None: # 确保有提示词内容
                self.update_template(target_template_name, system_prompt=new_system_prompt)
            else:
                logger.warning(f"No system_prompt content provided for template '{target_template_name}'.")

        elif event_id == EventBus.Buttons.RESET_MODEL_ARGS:
            logger.debug(f"ConfigManager handling RESET_MODEL_ARGS for default template.")
            target_template_name = self._current_config.default_config_name
            
            original_default_template = DEFAULT_AGENT_CONFIGS.get(target_template_name)
            if not original_default_template: # 如果目标名称不在预设中，则尝试 "default"
                original_default_template = DEFAULT_AGENT_CONFIGS.get("default")

            if original_default_template:
                # 获取原始默认配置的所有字段值进行重置
                # 注意：这里的 model_dump() ควรจะ包含所有字段，包括那些值为 None 的
                update_kwargs = original_default_template.model_dump(exclude_none=False) 
                # 通常重置模型参数时不应重置已挂载的MCP服务器，除非特别设计
                update_kwargs.pop('mounted_mcp_server_ids', None) 
                self.update_template(target_template_name, **update_kwargs)
            else:
                logger.error(f"Could not find original default configuration to reset template '{target_template_name}'.")


    def _handle_state_changed(self, event_data: dict):
        event_id = event_data.get("id")
        data = event_data.get("data") # 正确获取 data 字典

        if event_id == EventBus.States.MODEL_UPDATED:
            logger.debug(f"ConfigManager handling MODEL_UPDATED: {data}")
            target_template_name = self._current_config.default_config_name # 假设总是编辑默认模板
            
            kwargs_for_update = {
                "model": data.get("model"),
                "temperature": data.get("temperature"),
                "top_p": data.get("top_p"),
                "context_size": data.get("contexts_number"), 
                "max_tokens": data.get("max_tokens"), 
                "enable_streaming": data.get("streaming_output", False) # 确保有默认值
            }
            self.update_template(target_template_name, **kwargs_for_update)

        elif event_id == EventBus.States.MCP_SERVERS_UPDATED:
            logger.debug(f"ConfigManager handling MCP_SERVERS_UPDATED: {data}")
            target_template_name = self._current_config.default_config_name # 假设总是编辑默认模板
            selected_mcp_server_ids = [s["id"] for s in data.get("mcp_servers", [])]
            self.update_template(target_template_name, mounted_mcp_server_ids=selected_mcp_server_ids)
    
    def _load_config(self):
        """
        加载配置文件。
        如果文件不存在、内容为空、或加载/验证失败，则使用默认配置并尝试保存。
        """
        if os.path.exists(self._config_file_path):
            # 检查文件是否为空
            if os.path.getsize(self._config_file_path) == 0:
                logger.warning(f"Config file at {self._config_file_path} is empty. Using default config and saving it.")
                self._current_config = DEFAULT_APP_CONFIG.model_copy(deep=True)
                self.save_config(modified_template_name="default") # 保存默认配置
                return # 加载并保存后直接返回

            try:
                with open(self._config_file_path, 'r', encoding='utf-8') as f:
                    data = json.load(f)
                self._current_config = AppConfig.model_validate(data)
                logger.info(f"Config loaded from {self._config_file_path}")
            except (json.JSONDecodeError, ValidationError) as e:
                # 文件内容不是有效的JSON或不符合AppConfig结构
                logger.error(f"Error loading/validating config from {self._config_file_path}: {e}. Using default.")
                self._current_config = DEFAULT_APP_CONFIG.model_copy(deep=True) # 使用深拷贝
                self.save_config(modified_template_name="default") # 保存默认配置
            except Exception as e:
                # 其他意外错误
                logger.critical(f"Unexpected error loading config from {self._config_file_path}: {e}. Using default.")
                self._current_config = DEFAULT_APP_CONFIG.model_copy(deep=True) # 使用深拷贝
                self.save_config(modified_template_name="default") # 保存默认配置
        else:
            # 文件不存在
            logger.warning(f"Config file not found at {self._config_file_path}. Creating with default.")
            self._current_config = DEFAULT_APP_CONFIG.model_copy(deep=True) # 使用深拷贝
            self.save_config(modified_template_name="default") # 保存默认配置

    # save_config 可以接收一个参数，并在特定模板修改时发出事件
    def save_config(self, modified_template_name: Optional[str] = None):
        try:
            with open(self._config_file_path, 'w', encoding='utf-8') as f:
                f.write(self._current_config.model_dump_json(indent=4, exclude_none=False))
            logger.success(f"Config saved to {self._config_file_path}")

            if modified_template_name:
                logger.info(f"Emitting AGENT_TEMPLATE_UPDATED event for '{modified_template_name}'.")
                EventBus().publish(
                    EventBus.EventType.ConfigEvent,
                    {
                        "id": EventBus.ConfigEvents.AGENT_TEMPLATE_UPDATED,
                        "data": {"template_name": modified_template_name}
                    }
                )
        except Exception as e:
            logger.critical(f"Error saving config to {self._config_file_path}: {e}")

    def get_all_templates(self) -> Dict[str, AgentConfig]:
        return self._current_config.agent_templates

    def get_template(self, name: str) -> Optional[AgentConfig]:
        return self._current_config.agent_templates.get(name)

    def get_default_template(self) -> AgentConfig:
        default_name = self._current_config.default_config_name
        template = self._current_config.agent_templates.get(default_name)
        if template:
            return template
        
        logger.warning(f"Default template '{default_name}' not found. Falling back.")
        if "default" in self._current_config.agent_templates:
            self._current_config.default_config_name = "default"
            return self._current_config.agent_templates["default"]
        
        if self._current_config.agent_templates:
            first_template_name = next(iter(self._current_config.agent_templates.keys()))
            self._current_config.default_config_name = first_template_name
            return self._current_config.agent_templates[first_template_name]
        
        logger.error("No agent templates exist. Creating and returning a new 'default' template.")
        new_default_conf = DEFAULT_AGENT_CONFIGS["default"].model_copy(deep=True)
        self._current_config.agent_templates["default"] = new_default_conf
        self._current_config.default_config_name = "default"
        self.save_config(modified_template_name="default") # 保存这个新创建的默认模板并通知
        return new_default_conf

    def add_template(self, name: str, config: AgentConfig):
        self._current_config.agent_templates[name] = config
        self.save_config(modified_template_name=name) # 保存并通知
        logger.info(f"Agent template '{name}' added/updated.")

    def update_template(self, name: str, **kwargs):
        if name not in self._current_config.agent_templates:
            # 如果尝试更新默认模板但它不存在，则先创建它
            if name == self._current_config.default_config_name and name == "default":
                 logger.warning(f"Default template '{name}' does not exist. Creating it before update.")
                 self._current_config.agent_templates[name] = DEFAULT_AGENT_CONFIGS["default"].model_copy(deep=True)
            else:
                logger.warning(f"Template '{name}' does not exist. Cannot update.")
                return

        current_config_model = self._current_config.agent_templates[name]
        # 使用 model_copy(update=...) 进行安全更新，这会进行 Pydantic 验证
        try:
            # 过滤kwargs，只保留AgentConfig中实际存在的字段
            valid_kwargs = {k: v for k, v in kwargs.items() if k in AgentConfig.model_fields}
            updated_config = current_config_model.model_copy(update=valid_kwargs)
            self._current_config.agent_templates[name] = updated_config
            self.save_config(modified_template_name=name) # 保存并通知
            logger.success(f"Template '{name}' updated successfully with: {valid_kwargs}")
        except ValidationError as e:
            logger.error(f"Failed to update template '{name}' due to validation error: {e}")
        except Exception as e:
            logger.critical(f"Unexpected error updating template '{name}': {e}")

    def delete_template(self, name: str):
        if name in self._current_config.agent_templates:
            is_default_before_delete = (name == self._current_config.default_config_name)
            del self._current_config.agent_templates[name]
            
            if is_default_before_delete:
                logger.warning(f"Deleting current default template '{name}'. Setting a new default.")
                if self._current_config.agent_templates:
                    new_default_name = next(iter(self._current_config.agent_templates.keys()))
                    self.set_default_template(new_default_name) # 这会保存并发出 DEFAULT_AGENT_TEMPLATE_CHANGED
                else:
                    # 没有其他模板了，创建一个新的 "default" 模板
                    logger.info("No templates left. Creating a new 'default' template.")
                    new_default_conf = DEFAULT_AGENT_CONFIGS["default"].model_copy(deep=True)
                    self._current_config.agent_templates["default"] = new_default_conf
                    self.set_default_template("default") # 这会保存并发出 DEFAULT_AGENT_TEMPLATE_CHANGED
            else:
                self.save_config() # 仅保存，不针对此删除发出特定模板事件（因为LLMController主要关心默认模板）
            
            logger.success(f"Template '{name}' deleted successfully.")
        else:
            logger.warning(f"Template '{name}' not found.")

    def set_default_template(self, name: str):
        if name in self._current_config.agent_templates:
            if self._current_config.default_config_name == name:
                logger.info(f"Template '{name}' is already the default.")
                # 即使已经是默认，如果它的内容可能通过其他方式更改了，也可能需要通知
                # 但这里假设set_default只是改变名称引用
                # self.save_config() # 可能不需要，除非default_config_name的更改需要保存
                return

            self._current_config.default_config_name = name
            self.save_config() # 保存AppConfig中default_config_name的更改
            logger.success(f"Default template set to '{name}'.")
            EventBus().publish( # 发出默认模板已更改的事件
                EventBus.EventType.ConfigEvent,
                {
                    "id": EventBus.ConfigEvents.DEFAULT_AGENT_TEMPLATE_CHANGED,
                    "data": {"template_name": name}
                }
            )
        else:
            logger.warning(f"Template '{name}' not found. Cannot set as default.")

    def get_current_app_config(self) -> AppConfig:
        return self._current_config

    def get_all_mcp_servers(self) -> List[MCPServer]:
        return self._current_config.mcp_servers

    def get_mcp_server(self, server_id: str) -> Optional[MCPServer]:
        for server in self._current_config.mcp_servers:
            if server.id == server_id:
                return server
        return None

    def add_mcp_server(self, server: MCPServer):
        existing_server_index = -1
        for i, s in enumerate(self._current_config.mcp_servers):
            if s.id == server.id:
                existing_server_index = i
                break
        
        if existing_server_index != -1:
            self._current_config.mcp_servers[existing_server_index] = server
            logger.info(f"MCP server '{server.id}' updated.")
        else:
            self._current_config.mcp_servers.append(server)
            logger.info(f"MCP server '{server.id}' added.")
        self.save_config()

    def update_mcp_server(self, server_id: str, **kwargs):
        server_found = False
        for i, server in enumerate(self._current_config.mcp_servers):
            if server.id == server_id:
                updated_data = server.model_dump()
                updated_data.update(kwargs)
                try:
                    self._current_config.mcp_servers[i] = MCPServer(**updated_data)
                    self.save_config()
                    logger.success(f"MCP server '{server_id}' updated successfully.")
                    server_found = True
                    break
                except ValidationError as e:
                    logger.error(f"Failed to update MCP server '{server_id}': {e}")
        if not server_found:
            logger.warning(f"MCP server '{server_id}' not found.")


    def delete_mcp_server(self, server_id: str):
        original_len = len(self._current_config.mcp_servers)
        self._current_config.mcp_servers = [
            s for s in self._current_config.mcp_servers if s.id != server_id
        ]
        if len(self._current_config.mcp_servers) < original_len:
            logger.info(f"MCP server '{server_id}' deleted.")
            templates_modified = []
            for template_name, agent_template in self._current_config.agent_templates.items():
                if server_id in agent_template.mounted_mcp_server_ids:
                    agent_template.mounted_mcp_server_ids.remove(server_id)
                    logger.info(f"Removed reference to '{server_id}' from template '{template_name}'.")
                    templates_modified.append(template_name)
            
            self.save_config() # 先保存整体配置的更改
            # 然后为每个受影响的模板发出通知
            for tn in templates_modified:
                EventBus().publish(
                    EventBus.EventType.ConfigEvent,
                    {
                        "id": EventBus.ConfigEvents.AGENT_TEMPLATE_UPDATED,
                        "data": {"template_name": tn}
                    }
                )
        else:
            logger.warning(f"MCP server '{server_id}' not found.")

    def mount_mcp_server_to_template(self, template_name: str, server_id: str):
        template = self.get_template(template_name)
        if not template: # ... (error handling)
            return
        if self.get_mcp_server(server_id) is None: # ... (error handling)
            return

        if server_id not in template.mounted_mcp_server_ids:
            template.mounted_mcp_server_ids.append(server_id)
            self.save_config(modified_template_name=template_name) # 保存并通知
            logger.success(f"MCP server '{server_id}' mounted to template '{template_name}'.")
        else:
            logger.info(f"MCP server '{server_id}' already mounted to template '{template_name}'.")

    def unmount_mcp_server_from_template(self, template_name: str, server_id: str):
        template = self.get_template(template_name)
        if not template: # ... (error handling)
            return

        if server_id in template.mounted_mcp_server_ids:
            template.mounted_mcp_server_ids.remove(server_id)
            self.save_config(modified_template_name=template_name) # 保存并通知
            logger.success(f"MCP server '{server_id}' unmounted from template '{template_name}'.")
        else:
            logger.info(f"MCP server '{server_id}' is not mounted to template '{template_name}'.")

from pydantic_ai import Agent

# 辅助函数 create_agent_from_config 保持不变，当前主要由 LLMService 内部处理 Agent 创建
def create_agent_from_config(agent_config: AgentConfig) -> Agent:
    agent = Agent(
        model=agent_config.model,
        temperature=agent_config.temperature,
        top_p=agent_config.top_p,
        max_tokens=agent_config.max_tokens,
        system_prompt=agent_config.system_prompt,
        enable_streaming=agent_config.enable_streaming, # 确保传递
        **agent_config.model_kwargs 
    )
    return agent