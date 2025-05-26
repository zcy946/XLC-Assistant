from pydantic import BaseModel, Field, ValidationError
from typing import Optional, Dict, Any, Literal, List
from loguru import logger
import json
import os
from typing import Dict, Optional, List
from pydantic import ValidationError
from loguru import logger
from Singleton import Singleton 

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
        None, gt=0, description="模型可处理的最大 token 数，包括提示和补全内容。（仅作参考，PydanticAI 不直接设置）")

    model_kwargs: Dict[str, Any] = Field(
        {}, description="传递给底层 AI 模型的其他关键字参数。")

    enable_streaming: bool = Field(
        False, description="是否为 agent 启用流式输出。")

    # 新增：此模板选择挂载的 MCP 服务器的 ID 列表
    # 这些 ID 必须存在于 AppConfig 的 mcp_servers 列表中
    mounted_mcp_server_ids: List[str] = Field(
        default_factory=list, # 默认值为空列表
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
    
    # 新增：全局 MCP 服务器列表
    mcp_servers: List[MCPServer] = Field(
        default_factory=list, # 默认值为空列表
        description="所有已配置的 MCP 服务器列表。"
    )

    # Pydantic v2.x 的模型验证器
    # 确保 mounted_mcp_server_ids 中的 ID 存在于 mcp_servers 列表中
    # 注意：这个验证器在加载或创建 AppConfig 实例时运行
    def model_post_init(self, __context: Any) -> None:
        existing_mcp_ids = {server.id for server in self.mcp_servers}
        for template_name, agent_template in self.agent_templates.items():
            for server_id in agent_template.mounted_mcp_server_ids:
                if server_id not in existing_mcp_ids:
                    logger.warning(
                        f"Agent template '{template_name}' references unknown MCP server ID '{server_id}'. "
                        f"Please ensure all mounted_mcp_server_ids exist in the global mcp_servers list. "
                        f"Removing '{server_id}' from template '{template_name}'."
                    )
                    # 自动移除无效的引用，以避免后续错误
                    agent_template.mounted_mcp_server_ids.remove(server_id)
        # 确保 id 唯一的额外检查 (可选)
        if len(existing_mcp_ids) != len(self.mcp_servers):
            logger.warning("Duplicate MCP server IDs found in mcp_servers. Only unique IDs will be considered.")
            # 可以选择在这里清理重复的服务器，但保留原始顺序更复杂
            # self.mcp_servers = list({server.id: server for server in self.mcp_servers}.values())


# --- 默认配置 ---
DEFAULT_MCP_SERVERS = [
    MCPServer(
        id="my_local_mcp",
        name="本地开发MCP",
        url="http://localhost:8000/sse",
        api_key="dev_key_123",
        description="用于本地开发的MCP服务器。"
    )
]

DEFAULT_AGENT_CONFIGS = {
    "default": AgentConfig(
        model="deepseek:deepseek-chat",
        system_prompt="你是一个有用的助手，帮助用户完成任务。",
        mounted_mcp_server_ids=[] # 默认不挂载
    ),
    "creative": AgentConfig(
        model="deepseek:deepseek-chat",
        temperature=0.9,
        top_p=0.8,
        max_tokens=800,
        enable_streaming=True,
        system_prompt="你是一个极具创造力的故事讲述者和诗人。",
        mounted_mcp_server_ids=[]
    ),
    "precise": AgentConfig(
        model="deepseek:deepseek-chat",
        temperature=0.2,
        top_p=0.1,
        max_tokens=200,
        enable_streaming=False,
        system_prompt="你是一个简洁且注重事实的助手。提供直接的答案。",
        mounted_mcp_server_ids=["my_local_mcp"] # 示例：挂载一个默认服务器
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
        if config_file_path:
            self._config_file_path = config_file_path
        
        self._load_config()

    def _load_config(self):
        """
        从文件加载配置。如果文件不存在或加载失败，则使用默认配置。
        """
        if os.path.exists(self._config_file_path):
            try:
                with open(self._config_file_path, 'r', encoding='utf-8') as f:
                    data = json.load(f)
                    self._current_config = AppConfig.model_validate(data)
                    logger.info(f"Config loaded from {self._config_file_path}")
            except (json.JSONDecodeError, ValidationError) as e:
                logger.error(
                    f"Error loading or validating config from {self._config_file_path}: {e}")
                logger.info("Using default configuration.")
                self._current_config = DEFAULT_APP_CONFIG
            except Exception as e:
                logger.critical(
                    f"An unexpected error occurred while loading config: {e}")
                logger.info("Using default configuration.")
                self._current_config = DEFAULT_APP_CONFIG
        else:
            logger.warning(
                f"Config file not found at {self._config_file_path}. Creating with default configuration.")
            self._current_config = DEFAULT_APP_CONFIG
            self.save_config()  # 首次启动时保存默认配置

    def save_config(self):
        """
        保存当前配置到文件。
        """
        try:
            with open(self._config_file_path, 'w', encoding='utf-8') as f:
                f.write(self._current_config.model_dump_json(indent=4))
            logger.success(f"Config saved to {self._config_file_path}")
        except Exception as e:
            logger.critical(
                f"Error saving config to {self._config_file_path}: {e}")

    # --- Agent 模板管理方法 ---
    def get_all_templates(self) -> Dict[str, AgentConfig]:
        """
        获取所有可用的 Agent 配置模板。
        """
        return self._current_config.agent_templates

    def get_template(self, name: str) -> Optional[AgentConfig]:
        """
        根据名称获取一个 Agent 配置模板。
        """
        return self._current_config.agent_templates.get(name)

    def get_default_template(self) -> AgentConfig:
        """
        获取当前默认的 Agent 配置模板。
        """
        default_name = self._current_config.default_config_name
        template = self._current_config.agent_templates.get(default_name)
        if template:
            return template
        logger.warning(
            f"Default template '{default_name}' not found. Returning 'default' if available, otherwise first available.")
        if "default" in self._current_config.agent_templates:
            return self._current_config.agent_templates["default"]
        
        if self._current_config.agent_templates:
            return next(iter(self._current_config.agent_templates.values()))
        
        return AgentConfig(model="deepseek-chat", temperature=0.7) # 提供一个最基本的默认值

    def add_template(self, name: str, config: AgentConfig):
        """
        添加一个新的 Agent 配置模板。
        如果同名模板已存在，则会覆盖。
        """
        self._current_config.agent_templates[name] = config
        self.save_config()
        logger.info(f"Agent template '{name}' added/updated.")

    def update_template(self, name: str, **kwargs):
        """
        更新现有 Agent 配置模板的参数。
        """
        if name not in self._current_config.agent_templates:
            logger.warning(f"Template '{name}' does not exist.")
            return

        current_config = self._current_config.agent_templates[name]
        updated_data = current_config.model_dump()
        updated_data.update(kwargs)

        try:
            # 重新验证并创建 AgentConfig 实例以确保类型安全
            self._current_config.agent_templates[name] = AgentConfig(**updated_data)
            self.save_config()
            logger.success(f"Template '{name}' updated successfully.")
        except ValidationError as e:
            logger.error(
                f"Failed to update template '{name}' due to validation error: {e}")
        except Exception as e:
            logger.critical(
                f"An unexpected error occurred while updating template '{name}': {e}")

    def delete_template(self, name: str):
        """
        删除一个 Agent 配置模板。
        """
        if name in self._current_config.agent_templates:
            if name == self._current_config.default_config_name:
                logger.warning(
                    f"Deleting the current default template '{name}'. Please set a new default.")
                if len(self._current_config.agent_templates) > 1:
                    remaining_templates = [
                        k for k in self._current_config.agent_templates.keys() if k != name]
                    if remaining_templates:
                        self._current_config.default_config_name = remaining_templates[0]
                    else:
                        self._current_config.default_config_name = "default" 
                else:
                    self._current_config.default_config_name = "default" 

            del self._current_config.agent_templates[name]
            self.save_config()
            logger.success(f"Template '{name}' deleted successfully.")
        else:
            logger.warning(f"Template '{name}' not found.")

    def set_default_template(self, name: str):
        """
        设置新的默认配置模板。
        """
        if name in self._current_config.agent_templates:
            self._current_config.default_config_name = name
            self.save_config()
            logger.success(f"Default template set to '{name}'.")
        else:
            logger.warning(
                f"Template '{name}' not found. Cannot set as default.")

    def get_current_app_config(self) -> AppConfig:
        """
        获取整个应用程序的当前配置。
        """
        return self._current_config

    # --- MCP 服务器管理方法 ---
    def get_all_mcp_servers(self) -> List[MCPServer]:
        """
        获取所有已配置的 MCP 服务器。
        """
        return self._current_config.mcp_servers

    def get_mcp_server(self, server_id: str) -> Optional[MCPServer]:
        """
        根据 ID 获取一个 MCP 服务器。
        """
        for server in self._current_config.mcp_servers:
            if server.id == server_id:
                return server
        return None

    def add_mcp_server(self, server: MCPServer):
        """
        添加一个新的 MCP 服务器。如果 ID 已存在，则覆盖。
        """
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
        """
        更新现有 MCP 服务器的参数。
        """
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
                    logger.error(f"Failed to update MCP server '{server_id}' due to validation error: {e}")
                except Exception as e:
                    logger.critical(f"An unexpected error occurred while updating MCP server '{server_id}': {e}")
        if not server_found:
            logger.warning(f"MCP server '{server_id}' not found.")

    def delete_mcp_server(self, server_id: str):
        """
        删除一个 MCP 服务器。同时也会从所有 Agent 模板中移除对该服务器的引用。
        """
        original_len = len(self._current_config.mcp_servers)
        self._current_config.mcp_servers = [
            s for s in self._current_config.mcp_servers if s.id != server_id
        ]
        if len(self._current_config.mcp_servers) < original_len:
            logger.info(f"MCP server '{server_id}' deleted.")
            # 移除所有 Agent 模板中对该服务器的引用
            for template_name, agent_template in self._current_config.agent_templates.items():
                if server_id in agent_template.mounted_mcp_server_ids:
                    agent_template.mounted_mcp_server_ids.remove(server_id)
                    logger.info(f"Removed reference to '{server_id}' from template '{template_name}'.")
            self.save_config()
        else:
            logger.warning(f"MCP server '{server_id}' not found.")

    # --- Agent 模板挂载 MCP 服务器的方法 ---
    def mount_mcp_server_to_template(self, template_name: str, server_id: str):
        """
        为指定的 Agent 模板挂载一个 MCP 服务器。
        """
        template = self.get_template(template_name)
        if not template:
            logger.warning(f"Template '{template_name}' not found.")
            return

        if self.get_mcp_server(server_id) is None:
            logger.warning(f"MCP server '{server_id}' not found in global list. Cannot mount.")
            return

        if server_id not in template.mounted_mcp_server_ids:
            template.mounted_mcp_server_ids.append(server_id)
            self.save_config()
            logger.success(f"MCP server '{server_id}' mounted to template '{template_name}'.")
        else:
            logger.info(f"MCP server '{server_id}' already mounted to template '{template_name}'.")

    def unmount_mcp_server_from_template(self, template_name: str, server_id: str):
        """
        从指定的 Agent 模板中卸载一个 MCP 服务器。
        """
        template = self.get_template(template_name)
        if not template:
            logger.warning(f"Template '{template_name}' not found.")
            return

        if server_id in template.mounted_mcp_server_ids:
            template.mounted_mcp_server_ids.remove(server_id)
            self.save_config()
            logger.success(f"MCP server '{server_id}' unmounted from template '{template_name}'.")
        else:
            logger.info(f"MCP server '{server_id}' is not mounted to template '{template_name}'.")

# 辅助函数：创建 Agent 实例
from pydantic_ai import Agent

def create_agent_from_config(agent_config: AgentConfig) -> Agent:
    """
    根据 AgentConfig 创建一个 PydanticAI Agent 实例。
    """
    agent = Agent(
        model=agent_config.model,
        temperature=agent_config.temperature,
        top_p=agent_config.top_p,
        max_tokens=agent_config.max_tokens,
        system_prompt=agent_config.system_prompt,
        **agent_config.model_kwargs # 传递额外的模型参数
    )
    return agent