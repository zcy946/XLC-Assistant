#include "LLMService.h"
#include <QJsonObject>
#include "global.h"
#include <QNetworkReply>
#include "ToastManager.h"

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
    m_networkManager = new QNetworkAccessManager(this);
    connect(MCPService::getInstance(), &MCPService::sig_toolCallFinished, this, &LLMService::slot_onToolCallFinished);
}

void LLMService::postMessage(std::shared_ptr<Conversation> conversation, std::shared_ptr<Agent> agent, const mcp::json &tools, int max_retries)
{
    // 补全系统提示词
    if (!conversation->hasSystemPrompt())
    {
        conversation->resetSystemPrompt();
    }
    // 检查LLM是否存在
    std::shared_ptr<LLM> llm = DataManager::getInstance()->getLLM(agent->llmUUid);
    if (!llm)
    {
        XLC_LOG_DEBUG("Post message failed (conversationUuid={}, summary={}, agentUuid={}, agentName={}, llmUuid={}, toolsCount={}): LLM not found",
                      conversation->uuid,
                      conversation->summary,
                      agent->uuid,
                      agent->name,
                      llm->uuid,
                      tools.size());
        return;
    }

    // 构建请求体
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

    QNetworkRequest request(llm->baseUrl + llm->endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + llm->apiKey).toUtf8());
    // request.setTransferTimeout(120 * 1000); // 超时时间 120s

    QNetworkReply *reply = m_networkManager->post(request, QByteArray::fromStdString(body.dump()));
    connect(reply, &QNetworkReply::finished, this,
            [this, reply, conversation, agent, llm, tools, max_retries]()
            {
                handleResponse(reply, conversation, agent, llm, tools, max_retries - 1);
            });
    XLC_LOG_DEBUG("Posting message (conversationUuid={}, summary={}, agentUuid={}, agentName={}, llmUuid={}, modelID={}, modelName={}, toolsCount={}): \n{}",
                  conversation->uuid,
                  conversation->summary,
                  agent->uuid,
                  agent->name,
                  llm->uuid,
                  llm->modelID,
                  llm->modelName,
                  tools.size(),
                  body.dump(4));
}

void LLMService::handleResponse(QNetworkReply *reply, std::shared_ptr<Conversation> conversation, std::shared_ptr<Agent> agent, std::shared_ptr<LLM> llm, const mcp::json &tools, int retries_left)
{
    // 确保 reply 在处理完毕后被销毁
    reply->deleteLater();

    // 检查网络错误
    if (reply->error() != QNetworkReply::NoError)
    {
        QString errorMsg = QString("Post message failed (conversationUuid=%1, retries_left=%2), Network Error: %3")
                               .arg(conversation->uuid)
                               .arg(retries_left)
                               .arg(reply->errorString());
        XLC_LOG_WARN("{}", errorMsg);

        // 如果还有重试次数，则重试
        if (retries_left > 0)
        {
            postMessage(conversation, agent, tools, retries_left - 1);
        }
        else
        {
            QString finalErrorMsg = QString("Post message failed (conversationUuid=%1, baseURL=%2, endpoint=%3): failed to post to LLM after multiple retries")
                                        .arg(conversation->uuid)
                                        .arg(llm->baseUrl)
                                        .arg(llm->endpoint);
            XLC_LOG_ERROR("{}", finalErrorMsg);
            Q_EMIT sig_errorOccurred(conversation->uuid, finalErrorMsg);
        }
        return;
    }

    // 检查HTTP状态码
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 200)
    {
        try
        {
            QByteArray responseBody = reply->readAll();
            nlohmann::json json_data = nlohmann::json::parse(responseBody.toStdString());
            nlohmann::json responseMessage = json_data["choices"][0]["message"];

            XLC_LOG_DEBUG("Get response (conversationUuid={}): \n{}", conversation->uuid, responseMessage.dump(4));
            // 处理LLM响应
            handleSuccessfulResponse(conversation, responseMessage);
        }
        catch (const std::exception &e)
        {
            QString errorMsg = QString("Handle response failed (conversationUuid=%1, errorMsg=%2): failed to parse response")
                                   .arg(conversation->uuid)
                                   .arg(e.what());
            XLC_LOG_ERROR("{}", errorMsg);
            Q_EMIT sig_errorOccurred(conversation->uuid, errorMsg);
        }
    }
    else
    {
        // 处理非200的HTTP状态码
        QByteArray responseBody = reply->readAll();
        QString errorMsg = QString("Post message failed (conversationUuid=%1, retries_left=%2, statusCode=%3, body=%4)")
                               .arg(conversation->uuid)
                               .arg(retries_left)
                               .arg(statusCode)
                               .arg(QString::fromUtf8(responseBody));
        XLC_LOG_WARN("{}", errorMsg);
        // 如果还有重试次数，则重试
        if (retries_left > 0)
        {
            postMessage(conversation, agent, tools, retries_left - 1);
        }
        else
        {
            QString finalErrorMsg = QString("Post message failed (conversationUuid=%1, baseURL=%2, endpoint=%3): failed to post to LLM after multiple retries")
                                        .arg(conversation->uuid)
                                        .arg(llm->baseUrl)
                                        .arg(llm->endpoint);
            XLC_LOG_ERROR("{}", finalErrorMsg);
            Q_EMIT sig_errorOccurred(conversation->uuid, finalErrorMsg);
        }
    }
}

void LLMService::handleSuccessfulResponse(const std::shared_ptr<Conversation> &conversation, const nlohmann::json &responseMessage)
{
    QString content;
    if (!responseMessage.contains("content"))
        XLC_LOG_WARN("Handle response failed (conversationUuid={}): content not found in response", conversation->uuid);
    else if (!responseMessage["content"].is_null())
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
    Q_EMIT sig_responseReady(conversation->uuid, content.isEmpty() ? "tool_calls:\n" + toolCalls : content + "\ntool_calls:\n" + toolCalls);

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
        ToastManager::getInstance()->showMessage(Toast::Type::Warning, "Failed to handle ToolCallFinished event: Conversation not found");
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
        ToastManager::getInstance()->showMessage(Toast::Type::Warning, "Failed to handle ToolCallFinished event: Agent not found");
        return;
    }
    postMessage(conversation, agent, MCPService::getInstance()->getToolsFromServers(agent->mcpServers));
}