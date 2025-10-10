#include "LLMService.h"
#include <QJsonObject>
#include "global.h"
#include <QNetworkReply>
#include "ToastManager.h"
#include <QJsonDocument>

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

void LLMService::postMessage(std::shared_ptr<Conversation> conversation, std::shared_ptr<Agent> agent, const QJsonArray &tools, int max_retries)
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
        XLC_LOG_DEBUG("Post message failed (conversationUuid={}, summary={}, agentUuid={}, agentName={}, llmUuid={}, tools={}): LLM not found",
                      conversation->uuid,
                      conversation->summary,
                      agent->uuid,
                      agent->name,
                      llm->uuid,
                      QString::fromUtf8(QJsonDocument(tools).toJson(QJsonDocument::Indented)));
        return;
    }

    // 构建请求体
    QJsonObject jsonObjBody;
    if (!tools.empty())
    {
        jsonObjBody = {
            {"model", llm->modelID},
            {"max_tokens", agent->maxTokens},
            {"temperature", agent->temperature},
            {"messages", conversation->getCachedMessages()},
            {"tools", tools},
            {"tool_choice", "auto"}};
    }
    else
    {
        jsonObjBody = {
            {"model", llm->modelID},
            {"max_tokens", agent->maxTokens},
            {"temperature", agent->temperature},
            {"messages", conversation->getCachedMessages()}};
    }

    QNetworkRequest request(llm->baseUrl + llm->endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + llm->apiKey).toUtf8());
    // request.setTransferTimeout(120 * 1000); // 超时时间 120s

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(jsonObjBody).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this,
            [this, reply, conversation, agent, llm, tools, max_retries]()
            {
                handleResponse(reply, conversation, agent, llm, tools, max_retries - 1);
            });
    XLC_LOG_DEBUG("Posting message (conversationUuid={}, summary={}, agentUuid={}, agentName={}, llmUuid={}, modelID={}, modelName={}, toolsSize={}): \n{}",
                  conversation->uuid,
                  conversation->summary,
                  agent->uuid,
                  agent->name,
                  llm->uuid,
                  llm->modelID,
                  llm->modelName,
                  tools.size(),
                  QString::fromUtf8(QJsonDocument(jsonObjBody).toJson(QJsonDocument::Indented)));
}

void LLMService::handleResponse(QNetworkReply *reply, std::shared_ptr<Conversation> conversation, std::shared_ptr<Agent> agent, std::shared_ptr<LLM> llm, const QJsonArray &tools, int retries_left)
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
        // 序列化响应数据
        QJsonObject jsonObjResponse;
        QJsonParseError parseError;
        QJsonDocument jsonDocResponse = QJsonDocument::fromJson(reply->readAll(), &parseError);
        if (jsonDocResponse.isNull())
        {
            QString errorMsg = QString("Handle response failed, failed to parse response data to QJsonDocument (conversationUuid=%1): %2").arg(conversation->uuid).arg(parseError.errorString());
            XLC_LOG_ERROR("{}", errorMsg);
            Q_EMIT sig_errorOccurred(conversation->uuid, errorMsg);
            return;
        }
        // 解析LLM响应
        if (jsonDocResponse.isObject())
        {
            jsonObjResponse = jsonDocResponse.object();
            if (jsonObjResponse.contains("choices") && jsonObjResponse.value("choices").isArray())
            {
                QJsonArray jsonArrayChoices = jsonObjResponse.value("choices").toArray();
                QJsonObject jsonObjFirstChoice = jsonArrayChoices.first().toObject();
                if (jsonObjFirstChoice.contains("message") && jsonObjFirstChoice.value("message").isObject())
                {
                    QJsonObject jsonObjMessage = jsonObjFirstChoice.value("message").toObject();
                    handleSuccessfulResponse(conversation, jsonObjMessage);
                    XLC_LOG_DEBUG("Get response (conversationUuid={}): \n{}", conversation->uuid, QString::fromUtf8(QJsonDocument(jsonObjMessage).toJson(QJsonDocument::Indented)));
                    return;
                }
            }
        }
        // 解析失败
        QString errorMsg = QString("Handle response failed, invalid response data format (conversationUuid=%1): %2")
                               .arg(conversation->uuid)
                               .arg(QString::fromUtf8(jsonDocResponse.toJson(QJsonDocument::Indented)));
        XLC_LOG_ERROR("{}", errorMsg);
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

void LLMService::handleSuccessfulResponse(const std::shared_ptr<Conversation> &conversation, const QJsonObject &jsonObjMessage)
{
    QString content = QString();
    if (jsonObjMessage.contains("content"))
    {
        if (jsonObjMessage.value("content").isString())
            content = jsonObjMessage.value("content").toString();
    }
    else
    {
        XLC_LOG_WARN("Handle successful response warning (conversationUuid={}): content not found in response", conversation->uuid);
    }

    if (jsonObjMessage.contains("tool_calls") && jsonObjMessage.value("tool_calls").isArray())
    {
        // 需要调用工具
        QJsonArray jsonArrayToolCalls = jsonObjMessage.value("tool_calls").toArray();
        // 记录消息
        conversation->addMessage(Message(content, Message::ASSISTANT, getCurrentDateTime(), jsonArrayToolCalls));
        // 通知界面展示
        QString strToolCalls = QString::fromUtf8(QJsonDocument(jsonArrayToolCalls).toJson(QJsonDocument::Indented));
        Q_EMIT sig_responseReady(conversation->uuid, content.isEmpty() ? "tool_calls:\n" + strToolCalls : content + "\ntool_calls:\n" + strToolCalls);

        // 开始调用工具
        for (auto jsonValueToolCall : jsonArrayToolCalls)
        {
            if (jsonValueToolCall.isObject())
            {
                // 获取 callId
                QJsonObject jsonObjToolCall = jsonValueToolCall.toObject();
                if (jsonObjToolCall.contains("id") && jsonObjToolCall.value("id").isString())
                {
                    QString callId = jsonObjToolCall.value("id").toString();
                    if (jsonObjToolCall.contains("function") && jsonObjToolCall.value("function").isObject())
                    {
                        // 获取 callName
                        QJsonObject jsonObjFunction = jsonObjToolCall.value("function").toObject();
                        QString toolName = jsonObjFunction.value("name").toString();
                        // 展示调用过程
                        XLC_LOG_DEBUG("Calling tool (callId={}, toolName={})", callId, toolName);
                        Q_EMIT sig_toolCalled(conversation->uuid, QString("Calling tool (callId=%1, toolName=%2)").arg(callId).arg(toolName));
                        // 获取 arguments
                        QJsonObject jsonObjectArguments = QJsonObject();
                        if (jsonObjFunction.contains("arguments") && jsonObjFunction.value("arguments").isString())
                        {
                            QString strArguments = jsonObjFunction.value("arguments").toString();
                            jsonObjectArguments = QJsonDocument::fromJson(strArguments.toUtf8()).object();
                        }
                        // 执行工具
                        MCPService::getInstance()->callTool(CallToolArgs{conversation->uuid, callId, toolName, jsonObjectArguments});
                        // 更新待处理工具调用数量
                        conversation->pendingToolCalls += 1;
                        continue;
                    }
                }
            }
            XLC_LOG_WARN("Failed to call tool (conversationUuid={}): invalid tool call structure", conversation->uuid);
        }
    }
    else
    {
        // 记录消息
        conversation->addMessage(Message(content, Message::ASSISTANT, getCurrentDateTime()));
        // 通知界面展示
        Q_EMIT sig_responseReady(conversation->uuid, content);
    }
}

QString LLMService::formatMcpToolResponse(const QJsonObject &jsonObjToolCallResult, const QString &toolName, bool isVisionModel)
{
    QString content = "Here is the result of mcp tool use `" + toolName + "`:\n";

    if (jsonObjToolCallResult.contains("content") && jsonObjToolCallResult.value("content").isArray())
    {
        QJsonArray jsonArrayContent = jsonObjToolCallResult.value("content").toArray();

        if (isVisionModel)
        {
            for (int i = 0; i < jsonArrayContent.size(); ++i)
            {
                if (jsonArrayContent.at(i).isObject())
                {
                    QJsonObject jsonObjContentItem = jsonArrayContent.at(i).toObject();
                    if (jsonObjContentItem.contains("type") && jsonObjContentItem.value("type").isString())
                    {
                        QString type = jsonObjContentItem.value("type").toString();

                        if (type == "text")
                        {
                            QString text = "no content";
                            if (jsonObjContentItem.contains("text") && jsonObjContentItem.value("text").isString())
                            {
                                text = jsonObjContentItem.value("text").toString();
                            }
                            content += text + "\n";
                        }
                        else if (type == "image")
                        {
                            QString mimeType = "image/png";
                            if (jsonObjContentItem.contains("mimeType") && jsonObjContentItem.value("mimeType").isString())
                            {
                                mimeType = jsonObjContentItem.value("mimeType").toString();
                            }
                            QString data = "";
                            if (jsonObjContentItem.contains("data") && jsonObjContentItem.value("data").isString())
                            {
                                data = jsonObjContentItem.value("data").toString();
                            }
                            content += "Here is a image result: data:" + mimeType + ";base64," + data + "\n";
                        }
                        else if (type == "audio")
                        {
                            QString mimeType = "audio/mp3";
                            if (jsonObjContentItem.contains("mimeType") && jsonObjContentItem.value("mimeType").isString())
                            {
                                mimeType = jsonObjContentItem.value("mimeType").toString();
                            }
                            QString data = "";
                            if (jsonObjContentItem.contains("data") && jsonObjContentItem.value("data").isString())
                            {
                                data = jsonObjContentItem.value("data").toString();
                            }
                            content += "Here is a audio result: data:" + mimeType + ";base64," + data + "\n";
                        }
                        else
                        {
                            content += "Here is a unsupported result type: " + type + "\n";
                        }
                    }
                }
            }
        }
        else
        {
            // 对于非视觉模型，将整个数组序列化为 JSON 字符串
            content += QString::fromUtf8(QJsonDocument(jsonArrayContent).toJson(QJsonDocument::Compact)) + "\n";
        }
    }
    else
    {
        content += "no content\n";
    }

    return content;
};

void LLMService::slot_onToolCallFinished(const CallToolArgs &callToolArgs, bool success, const QJsonObject &jsonObjectToolCallResult, const QString &errorMessage)
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
        QString formattedContent = formatMcpToolResponse(jsonObjectToolCallResult, callToolArgs.toolName, false);
        conversation->addMessage(Message(formattedContent, Message::TOOL, getCurrentDateTime(), QJsonArray(), callToolArgs.callId));

        // 展示调用结果
        Q_EMIT sig_toolCalled(conversation->uuid, QString("Result of call tool (success=%1, callId=%2, formattedContent=%3)")
                                                      .arg(success)
                                                      .arg(callToolArgs.callId)
                                                      .arg(formattedContent));
    }
    else
    {
        if (!conversation)
        {
            XLC_LOG_WARN("Conversation not found (conversationUuid={})", callToolArgs.conversationUuid);
            return;
        }
        conversation->addMessage(Message(errorMessage, Message::TOOL, getCurrentDateTime(), QJsonArray(), callToolArgs.callId));

        // 展示调用结果
        Q_EMIT sig_toolCalled(conversation->uuid, QString("Result of call tool (success=%1, callId=%2, errorMessage=%3)")
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