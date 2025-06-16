import json
import random

def assign_random_mcp_servers(agents_file="Agents.json", mcpservers_file="MCPServers.json"):
    """
    为 Agents.json 中的每个 Agent 随机添加 0-10 个 MCPServers.json 中的 UUID。
    """
    try:
        # 1. 读取 MCPServers.json 并提取 UUID
        with open(mcpservers_file, 'r', encoding='utf-8') as f:
            mcp_servers_data = json.load(f)
        
        mcp_server_uuids = [server['uuid'] for server in mcp_servers_data if 'uuid' in server]
        
        if not mcp_server_uuids:
            print(f"Warning: No UUIDs found in {mcpservers_file}. No mcpServers will be assigned.")
            return

        # 2. 读取 Agents.json
        with open(agents_file, 'r', encoding='utf-8') as f:
            agents_data = json.load(f)

        # 3. 遍历 Agents 列表并修改
        for agent in agents_data:
            num_servers_to_add = random.randint(0, 10) # 随机生成 0 到 10 之间的一个数
            
            # 确保 mcpServers 键存在且是列表
            if 'mcpServers' not in agent or not isinstance(agent['mcpServers'], list):
                agent['mcpServers'] = []
            
            # 清空现有列表，重新添加（如果不想清空，可以保留原有数据再append）
            agent['mcpServers'].clear() 

            # 从所有 UUID 中随机选择
            selected_uuids = random.sample(mcp_server_uuids, min(num_servers_to_add, len(mcp_server_uuids)))
            
            agent['mcpServers'].extend(selected_uuids)
            
            print(f"Agent '{agent.get('name', 'Unnamed')}' assigned {len(selected_uuids)} mcpServers.")

        # 4. 将修改后的数据写回 Agents.json
        with open(agents_file, 'w', encoding='utf-8') as f:
            json.dump(agents_data, f, indent=4, ensure_ascii=False) # indent=4 使输出更可读

        print(f"\nSuccessfully updated {agents_file}")

    except FileNotFoundError as e:
        print(f"Error: One of the files not found. {e}")
    except json.JSONDecodeError as e:
        print(f"Error: Invalid JSON format in file. {e}")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")

if __name__ == "__main__":
    assign_random_mcp_servers()