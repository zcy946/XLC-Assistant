#include "LLMService.h"
#include <QtConcurrent>
#include "global.h"
#include "DataManager.h"

LLMService::LLMService(QObject *parent)
    : QObject(parent), m_maxMcpToolChainCall(3)
{
}

void LLMService::processRequest(const std::shared_ptr<Conversation> &conversation, const std::shared_ptr<Agent> &agent, const mcp::json &tools, int max_retries)
{
    // 使用QtConcurrent::run来在后台线程执行耗时操作
    QtConcurrent::run(
        [this, conversation, agent, tools, max_retries]()
        {
            const std::shared_ptr<LLM> &llm = DataManager::getInstance()->getLLM(agent->llmUUid);
            XLC_LOG_TRACE("from conversation[{}]: {}\n\tagent: \n\t\t{}\n\t\t{}\n\tllm: \n\t\t{}\n\t\t{}\n\t\t{}\n\ttools: \n\t\t{}", conversation->uuid, conversation->summary, agent->uuid, agent->name, llm->uuid, llm->modelID, llm->modelName, tools.dump(4));
            nlohmann::json body =
                {
                    {"model", llm->modelID.toStdString()},
                    {"max_tokens", agent->maxTokens},
                    {"temperature", agent->temperature},
                    {"messages", conversation->messages},
                    {"tools", tools},
                    {"tool_choice", "auto"}};
            m_client = std::make_unique<httplib::Client>(llm->baseUrl.toStdString());
            m_client->set_default_headers({{"Authorization", "Bearer " + std::string(llm->apiKey.toStdString())}});
            m_client->set_connection_timeout(10);

            int retry = 0;
            while (retry <= max_retries)
            {
                auto res = m_client->Post(llm->endpoint.toStdString(), body.dump(), "application/json");

                if (res && res->status == 200)
                {
                    try
                    {
                        nlohmann::json json_data = nlohmann::json::parse(res->body);
                        nlohmann::json message = json_data["choices"][0]["message"];

                        // 将 nlohmann::json 转换为 QString 以便信号传递
                        QString responseStr = QString::fromStdString(message.dump());
                        XLC_LOG_TRACE("AI: {}", responseStr);
                        emit responseReady(conversation->uuid, responseStr);
                        /**
                         * TODO 这个信号的槽函数应该在获取到响应后，应使用 m_maxMcpToolChainCall 值循环，查看是否需要调用函数
                         *  - 如果不需要则break - 存储记录
                         *  - 如果需要则调用所有工具 - 存储各个记录 - (m_maxMcpToolChainCall - 1) - 继续调用processRequest*/
                        return;
                    }
                    catch (const std::exception &e)
                    {
                        QString errorMsg = QString("Failed to parse response: %1").arg(e.what());
                        emit errorOccurred(conversation->uuid, errorMsg);
                    }
                }
                else
                {
                    QString errorMsg = "Failed to get response from LLM. ";
                    if (res)
                    {
                        errorMsg += QString("Status: %1, Body: %2").arg(res->status).arg(QString::fromStdString(res->body));
                    }
                    else
                    {
                        errorMsg += QString("Error: %1").arg(httplib::to_string(res.error()).c_str());
                    }
                    XLC_LOG_ERROR("{}", errorMsg);
                    emit errorOccurred(conversation->uuid, errorMsg);
                }
                retry++;
            }
        });
}
