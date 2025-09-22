#include "LLMService.h"
#include <QtConcurrent>
#include "global.h"
#include "DataManager.h"
#include "MCPService.h"

LLMService *LLMService::s_instance = nullptr;

LLMService *LLMService::getInstance()
{
    if (!s_instance)
    {
        s_instance = new LLMService();
        // 在应用程序退出时自动清理单例实例
        connect(qApp, &QCoreApplication::aboutToQuit, s_instance, &QObject::deleteLater);
    }
    return s_instance;
}

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
            XLC_LOG_TRACE("Processing request (conversationUuid={}, summary={}, agentUuid={}, agentName={}, llmUuid={}, modelID={}, modelName={}, tools={})",
                          conversation->uuid,
                          conversation->summary,
                          agent->uuid,
                          agent->name,
                          llm->uuid,
                          llm->modelID,
                          llm->modelName,
                          tools.dump(4));
            nlohmann::json body;
            if (tools.is_array() && !tools.empty())
            {
                body = {
                    {"model", llm->modelID.toStdString()},
                    {"max_tokens", agent->maxTokens},
                    {"temperature", agent->temperature},
                    {"messages", conversation->messages},
                    {"tools", tools},
                    {"tool_choice", "auto"}};
            }
            else
            {
                body = {
                    {"model", llm->modelID.toStdString()},
                    {"max_tokens", agent->maxTokens},
                    {"temperature", agent->temperature},
                    {"messages", conversation->messages}};
            }
            m_client = std::make_unique<httplib::Client>(llm->baseUrl.toStdString());
            m_client->set_default_headers({{"Authorization", "Bearer " + std::string(llm->apiKey.toStdString())}});
            m_client->set_connection_timeout(10);

            for (int retry = 0; retry <= max_retries; ++retry)
            {
                XLC_LOG_TRACE("Posting to LLM (baseURL={}, endpoint={}, body={})", llm->baseUrl, llm->endpoint, body.dump(4));
                httplib::Result result = m_client->Post(llm->endpoint.toStdString(), body.dump(), "application/json");
                if (result && result->status == 200)
                {
                    try
                    {
                        nlohmann::json json_data = nlohmann::json::parse(result->body);
                        nlohmann::json responseMessage = json_data["choices"][0]["message"];

                        // 将 nlohmann::json 转换为 QString 以便信号传递
                        XLC_LOG_TRACE("Request response (conversationUuid={}, response={})", conversation->uuid, responseMessage.dump(4));
                        // 处理LLM响应
                        processResponse(conversation, responseMessage);
                        // emit sig_responseReady(conversation->uuid, responseStr);
                    }
                    catch (const std::exception &e)
                    {
                        QString errorMsg = QString("Process request failed (conversationUuid=%1, errorMsg=%2): failed to parse response")
                                               .arg(conversation->uuid)
                                               .arg(e.what());
                        emit sig_errorOccurred(conversation->uuid, errorMsg);
                    }
                    return;
                }
                else
                {
                    QString errorMsg = QString("Post LLM failed (conversationUuid=%1, retries=%2)")
                                           .arg(conversation->uuid)
                                           .arg(retry + 1);
                    if (result)
                    {
                        errorMsg += QString(" Status: %1, Body: %2").arg(result->status).arg(QString::fromStdString(result->body));
                    }
                    else
                    {
                        errorMsg += QString(" Error: %1").arg(httplib::to_string(result.error()).c_str());
                    }
                    XLC_LOG_WARN("{}", errorMsg);
                }
            }
            QString errorMsg = QString("Process request failed (conversationUuid=%1, retries=%2, baseURL=%3, endpoint=%4): failed to post to LLM after multiple retries")
                                   .arg(conversation->uuid)
                                   .arg(max_retries)
                                   .arg(llm->baseUrl)
                                   .arg(llm->endpoint);
            XLC_LOG_ERROR("{}", errorMsg);
            emit sig_errorOccurred(conversation->uuid, errorMsg);
        });
}

void LLMService::processResponse(const std::shared_ptr<Conversation> &conversation, const nlohmann::json &responseMessage)
{
    // 记录回答
    conversation->messages.push_back(responseMessage);

    // 没有调用工具
    // if (responseMessage["tool_calls"].empty())
    if (!responseMessage.contains("tool_calls"))
    {
        // TODO 展示结果
        return;
    }

    // 调用了工具
    for (const auto &tool_call : responseMessage["tool_calls"])
    {
        QString callId = QString::fromStdString(tool_call["id"].get<std::string>());
        QString toolName = QString::fromStdString(tool_call["function"]["name"].get<std::string>());
        // TODO 展示调用过程
        XLC_LOG_DEBUG("Calling tool (callId={}, toolName={})", callId, toolName);
        // 解析参数
        mcp::json args = tool_call["function"]["arguments"];
        if (args.is_string())
        {
            args = mcp::json::parse(args.get<std::string>());
        }
        // 执行工具
        MCPService::getInstance()->callTool(CallToolArgs{conversation->uuid, callId, toolName, args});
        // TODO 处理调用结果连接MCPService的sig_toolCallFinished信号，通过conversation->uuid分类
    }
}