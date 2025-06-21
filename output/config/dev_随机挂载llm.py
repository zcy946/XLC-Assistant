import json
import random

# Load Agents.json
with open('./Agents.json', 'r', encoding='utf-8') as f:
    agents_data = json.load(f)

# Load LLMs.json
with open('./LLMs.json', 'r', encoding='utf-8') as f:
    llms_data = json.load(f)

# Extract all UUIDs from LLMs.json
llm_uuids = [llm['uuid'] for llm in llms_data]

# Update modelUuid for each agent
for agent in agents_data:
    agent['modelUuid'] = random.choice(llm_uuids)

# Save the modified Agents.json
with open('Agents.json', 'w', encoding='utf-8') as f:
    json.dump(agents_data, f, indent=4, ensure_ascii=False)

print("Agents.json has been updated successfully with random modelUuids from LLMs.json.")