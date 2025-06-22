#include "LLMService.h"
#include <QtConcurrent>
#include "global.h"
#include "DataManager.h"

LLMService::LLMService(QObject *parent)
    : QObject(parent), m_maxMcpToolChainCall(3)
{
}

void LLMService::processRequest(const QString &conversationUuid, const std::shared_ptr<Agent> &agent, const mcp::json &messages, const mcp::json &tools, int max_retries)
{
    // 使用QtConcurrent::run来在后台线程执行耗时操作
    QtConcurrent::run(
        [this, conversationUuid, agent, messages, tools, max_retries]()
        {
            nlohmann::json body =
                {
                    {"model", agent->llmUUid.toStdString()},
                    {"max_tokens", agent->maxTokens},
                    {"temperature", agent->temperature},
                    {"messages", messages},
                    {"tools", tools},
                    {"tool_choice", "auto"}};
            m_client = std::make_unique<httplib::Client>(BASE_URL);
            m_client->set_default_headers({{"Authorization", "Bearer " + std::string(API_KEY)}});
            m_client->set_connection_timeout(10);

            int retry = 0;
            while (retry <= max_retries)
            {
                auto res = m_client->Post(ENDPOINT, body.dump(), "application/json");

                if (res && res->status == 200)
                {
                    try
                    {
                        nlohmann::json json_data = nlohmann::json::parse(res->body);
                        nlohmann::json message = json_data["choices"][0]["message"];

                        // 将 nlohmann::json 转换为 QString 以便信号传递
                        QString responseStr = QString::fromStdString(message.dump());
                        emit responseReady(conversationUuid, responseStr); 
                        /**
                         * TODO 这个信号的槽函数应该在获取到响应后，应使用 m_maxMcpToolChainCall 值循环，查看是否需要调用函数
                         *  - 如果不需要则break - 存储记录
                         *  - 如果需要则调用所有工具 - 存储各个记录 - (m_maxMcpToolChainCall - 1) - 继续调用processRequest*/ 
                        return;
                    }
                    catch (const std::exception &e)
                    {
                        QString errorMsg = QString("Failed to parse response: %1").arg(e.what());
                        emit errorOccurred(conversationUuid, errorMsg);
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
                    emit errorOccurred(conversationUuid, errorMsg);
                }
                retry++;
            }
        });
}
