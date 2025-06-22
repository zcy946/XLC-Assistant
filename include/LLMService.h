#ifndef LLMSERVICE_H
#define LLMSERVICE_H

#include <QObject>
#include "DataManager.h"
#include <mcp_message.h>
#include <httplib.h>

class LLMService : public QObject
{
    Q_OBJECT
Q_SIGNALS:
    void responseReady(const QString &conversationUuid, const QString &responseJson);
    void errorOccurred(const QString &conversationUuid, const QString &errorMessage);

public:
    explicit LLMService(QObject *parent = nullptr);
    ~LLMService() = default;
    /**
     * @brief 向指定agent发送消息.
     *
     * 此函数向给定agent发送 JSON 格式的消息。如果操作失败，它将自动重试达到指定次数。
     *
     * @param agent 指向将接收消息的 Agent 对象的共享指针.
     * @param messages 要发送消息的 JSON 对象.
     * @param tools 可使用的工具列表.
     * @param max_retries 失败时最大重试次数（默认为3）.
     */
    void processRequest(const QString &conversationUuid, const std::shared_ptr<Agent> &agent, const mcp::json &messages, const mcp::json &tools, int max_retries = 3);

private:
    /**
     * @brief 链式调用次数.
     *
     * 此变量限制“思考-行动”循环中的最大迭代次数，
     * 其中“行动”指LLM决定调用外部工具来解决问题。
     */
    int m_maxMcpToolChainCall;
    std::unique_ptr<httplib::Client> m_client;
};

#endif // LLMSERVICE_H