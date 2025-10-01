#ifndef LLMSERVICE_H
#define LLMSERVICE_H

#include <QObject>
#include <mcp_message.h>
#include <QNetworkAccessManager>
#include "MCPService.h"

struct Agent;
struct Conversation;
class LLMService : public QObject
{
    Q_OBJECT
Q_SIGNALS:
    void sig_responseReady(const QString &conversationUuid, const QString &responseMessage);
    void sig_errorOccurred(const QString &conversationUuid, const QString &errorMessage);
    void sig_toolCalled(const QString &conversationUuid, const QString &message);

private Q_SLOTS:
    void slot_onToolCallFinished(const CallToolArgs &callToolArgs, bool success, const mcp::json &result, const QString &errorMessage);

public:
    static LLMService *getInstance();
    ~LLMService() = default;
    /**
     * @brief 向指定agent发送消息.
     *
     * 此函数向给定agent发送 JSON 格式的消息。如果操作失败，它将自动重试达到指定次数。
     *
     * @param agent 指向将接收消息的 Agent 对象的共享指针.
     * @param messages 要发送消息的 JSON 对象.
     * @param tools 可使用的工具列表（默认为空）.
     * @param max_retries 失败时最大重试次数（默认为3）.
     */
    void postMessage(std::shared_ptr<Conversation> conversation, std::shared_ptr<Agent> agent, const mcp::json &tools = mcp::json(), int max_retries = 3);

private:
    explicit LLMService(QObject *parent = nullptr);
    LLMService(const LLMService &) = delete;
    LLMService &operator=(const LLMService &) = delete;
    void handleResponse(QNetworkReply *reply, std::shared_ptr<Conversation> conversation, std::shared_ptr<Agent> agent, std::shared_ptr<LLM> llm, const mcp::json &tools, int retries_left);
    void handleSuccessfulResponse(const std::shared_ptr<Conversation> &conversation, const nlohmann::json &responseMessage);
    std::string formatMcpToolResponse(const mcp::json &result, const std::string &toolName, bool isVisionModel = false);

private:
    static LLMService *s_instance;
    QNetworkAccessManager *m_networkManager;
};

#endif // LLMSERVICE_H