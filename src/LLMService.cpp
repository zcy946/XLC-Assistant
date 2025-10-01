#include "LLMService.h"
#include <QtConcurrent>
#include "global.h"

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
    : QObject(parent)
{
    connect(MCPService::getInstance(), &MCPService::sig_toolCallFinished, this, &LLMService::slot_onToolCallFinished);
}

void LLMService::processRequest(std::shared_ptr<Conversation> conversation, std::shared_ptr<Agent> agent, const mcp::json &tools, int max_retries)
{
    // 使用QtConcurrent::run来在后台线程执行耗时操作
    QtConcurrent::run(
        [this, conversation, agent, tools, max_retries]()
        {
            // 检查系统提示词
            if (!conversation->hasSystemPrompt())
            {
                conversation->resetSystemPrompt();
            }
            std::shared_ptr<LLM> llm = DataManager::getInstance()->getLLM(agent->llmUUid);
            XLC_LOG_DEBUG("Processing request (conversationUuid={}, summary={}, agentUuid={}, agentName={}, llmUuid={}, modelID={}, modelName={}, toolsCount={})",
                          conversation->uuid,
                          conversation->summary,
                          agent->uuid,
                          agent->name,
                          llm->uuid,
                          llm->modelID,
                          llm->modelName,
                          tools.size());
            nlohmann::json body;
            if (tools.is_array() && !tools.empty())
            {
                body = {
                    {"model", llm->modelID.toStdString()},
                    {"max_tokens", agent->maxTokens},
                    {"temperature", agent->temperature},
                    {"messages", conversation->getCachedMessages()},
                    {"tools", tools},
                    {"tool_choice", "auto"}};
            }
            else
            {
                body = {
                    {"model", llm->modelID.toStdString()},
                    {"max_tokens", agent->maxTokens},
                    {"temperature", agent->temperature},
                    {"messages", conversation->getCachedMessages()}};
            }
            // TODO 使用Qt的http库重构代码
            std::unique_ptr<httplib::Client> m_client = std::make_unique<httplib::Client>(llm->baseUrl.toStdString());
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

                        XLC_LOG_TRACE("Request response (conversationUuid={}, response={})", conversation->uuid, responseMessage.dump(4));
                        // 处理LLM响应
                        processResponse(conversation, responseMessage);
                    }
                    catch (const std::exception &e)
                    {
                        QString errorMsg = QString("Process request failed (conversationUuid=%1, errorMsg=%2): failed to parse response")
                                               .arg(conversation->uuid)
                                               .arg(e.what());
                        XLC_LOG_ERROR("{}", errorMsg);
                        Q_EMIT sig_errorOccurred(conversation->uuid, errorMsg);
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
            Q_EMIT sig_errorOccurred(conversation->uuid, errorMsg);
        });
}

void LLMService::processResponse(const std::shared_ptr<Conversation> &conversation, const nlohmann::json &responseMessage)
{
    QString content;
    if (!responseMessage.contains("content"))
        XLC_LOG_WARN("process response failed (conversationUuid={}): content not found in response", conversation->uuid);
    else
        content = QString::fromStdString(responseMessage.value("content", ""));

    // 没有调用工具
    if (!responseMessage.contains("tool_calls"))
    {
        // 记录消息
        conversation->addMessage(Message(content, Message::ASSISTANT, getCurrentDateTime()));
        // 通知界面展示
        Q_EMIT sig_responseReady(conversation->uuid, content);
        return;
    }

    // 调用了工具
    QString toolCalls = QString::fromStdString(responseMessage["tool_calls"].dump());
    // 记录消息
    conversation->addMessage(Message(content, Message::ASSISTANT, getCurrentDateTime(), toolCalls));
    // 通知界面展示
    Q_EMIT sig_responseReady(conversation->uuid, content + "\ntool_calls:\n" + toolCalls);

    // 开始调用工具
    for (const auto &tool_call : responseMessage["tool_calls"])
    {
        QString callId = QString::fromStdString(tool_call["id"].get<std::string>());
        QString toolName = QString::fromStdString(tool_call["function"]["name"].get<std::string>());
        // 展示调用过程
        XLC_LOG_DEBUG("Calling tool (callId={}, toolName={})", callId, toolName);
        Q_EMIT sig_toolCalled(conversation->uuid, QString::fromStdString("Calling tool (callId=%1, toolName=%2)").arg(callId).arg(toolName));
        // 解析参数
        mcp::json args = tool_call["function"]["arguments"];
        if (args.is_string())
        {
            args = mcp::json::parse(args.get<std::string>());
        }
        // 执行工具
        MCPService::getInstance()->callTool(CallToolArgs{conversation->uuid, callId, toolName, args});
        // 更新待处理工具调用数量
        conversation->pendingToolCalls += 1;
    }
}

std::string LLMService::formatMcpToolResponse(const mcp::json &result, const std::string &toolName, bool isVisionModel)
{
    std::string content = "Here is the result of mcp tool use `" + toolName + "`:\n";

    if (result.contains("content") && result["content"].is_array())
    {
        const auto &contentArray = result["content"];

        if (isVisionModel)
        {
            for (const auto &item : contentArray)
            {
                if (item.contains("type"))
                {
                    std::string type = item["type"];

                    if (type == "text")
                    {
                        std::string text = item.contains("text") ? item["text"] : "no content";
                        content += text + "\n";
                    }
                    else if (type == "image")
                    {
                        std::string mimeType = item.contains("mimeType") ? item["mimeType"] : "image/png";
                        std::string data = item.contains("data") ? item["data"] : "";
                        content += "Here is a image result: data:" + mimeType + ";base64," + data + "\n";
                    }
                    else if (type == "audio")
                    {
                        std::string mimeType = item.contains("mimeType") ? item["mimeType"] : "audio/mp3";
                        std::string data = item.contains("data") ? item["data"] : "";
                        content += "Here is a audio result: data:" + mimeType + ";base64," + data + "\n";
                    }
                    else
                    {
                        content += "Here is a unsupported result type: " + type + "\n";
                    }
                }
            }
        }
        else
        {
            // 对于非视觉模型，将整个数组序列化为 JSON 字符串
            content += contentArray.dump() + "\n";
        }
    }
    else
    {
        content += "no content\n";
    }

    return content;
};

void LLMService::slot_onToolCallFinished(const CallToolArgs &callToolArgs, bool success, const mcp::json &result, const QString &errorMessage)
{
    std::shared_ptr<Conversation> conversation = DataManager::getInstance()->getConversation(callToolArgs.conversationUuid);
    if (!conversation)
    {
        // TODO 处理异常错误
        XLC_LOG_WARN("Conversation not found (conversationUuid={})", callToolArgs.conversationUuid);
        return;
    }

    if (success)
    {
        // 更新消息列表
        std::string formattedContent = formatMcpToolResponse(result, callToolArgs.toolName.toStdString(), false);
        conversation->addMessage(Message(QString::fromStdString(formattedContent), Message::TOOL, getCurrentDateTime(), QString(), callToolArgs.callId));

        // 展示调用结果
        Q_EMIT sig_toolCalled(conversation->uuid, QString::fromStdString("Result of call tool (success=%1, callId=%2, formattedContent=%3)")
                                                      .arg(success)
                                                      .arg(callToolArgs.callId)
                                                      .arg(QString::fromStdString(formattedContent)));
    }
    else
    {
        if (!conversation)
        {
            XLC_LOG_WARN("Conversation not found (conversationUuid={})", callToolArgs.conversationUuid);
            return;
        }
        conversation->addMessage(Message(errorMessage, Message::TOOL, getCurrentDateTime(), Q_NULLPTR, callToolArgs.callId));

        // 展示调用结果
        Q_EMIT sig_toolCalled(conversation->uuid, QString::fromStdString("Result of call tool (success=%1, callId=%2, errorMessage=%3)")
                                                      .arg(success)
                                                      .arg(callToolArgs.callId)
                                                      .arg(errorMessage));
    }
    // 更新待处理工具调用数量
    conversation->pendingToolCalls -= 1;

    // 保证所有toolcall获取到结果后再响应LLM
    if (conversation->pendingToolCalls > 0)
        return;
    std::shared_ptr<Agent> agent = DataManager::getInstance()->getAgent(conversation->agentUuid);
    if (!agent)
    {
        // TODO 处理异常错误
        XLC_LOG_WARN("Agent not found (conversationUuid={}, agentUuid={})", conversation->uuid, conversation->agentUuid);
        return;
    }
    processRequest(conversation, agent, MCPService::getInstance()->getToolsFromServers(agent->mcpServers));
}