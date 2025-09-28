#ifndef GLOBAL_H
#define GLOBAL_H

#include <QFont>
#include <QFontMetrics>
#include <QApplication>
#include <QVector>
#include <QVariant>
#include <QDateTime>
#include <QUuid>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>

constexpr const char *FILE_LLMS = "./config/LLMs.json";
constexpr const char *FILE_MCPSERVERS = "./config/MCPServers.json";
constexpr const char *FILE_AGENTS = "./config/Agents.json";
constexpr const char *BASE_URL = "https://api.deepseek.com";
constexpr const char *ENDPOINT = "/v1/chat/completions";
constexpr const char *MODEL = "deepseek-chat";
constexpr const char *API_KEY = "sk-67827bd147dc43afbb9a982349c4be31";

constexpr const char *AVATAR_UNKNOW = "://image/avatar_unknow.png";
constexpr const char *AVATAR_TOOL = "://image/avatar_tool.png";
constexpr const char *AVATAR_SYSTEM = "://image/avatar_system.png";
constexpr const char *DEFAULT_AVATAR_USER = "://image/default_avatar_user.png";
constexpr const char *DEFAULT_AVATAR_LLM = "://image/default_avatar_llm.png";

QString getCurrentDateTime();
QString generateUuid();
struct Message
{
    QString id;
    QString text;
    enum Role
    {
        USER = 0,
        ASSISTANT = 1,
        TOOL = 2,
        SYSTEM = 3,
        UNKNOWN = 4
    };
    Role role;
    QString createdTime;
    QString avatarFilePath;
    QString toolCalls;
    QString toolCallId;

    Message()
        : id(generateUuid()), createdTime(getCurrentDateTime())
    {
        switch (role)
        {
        case Role::USER:
            this->avatarFilePath = QString(DEFAULT_AVATAR_USER);
            break;
        case Role::ASSISTANT:
            this->avatarFilePath = QString(DEFAULT_AVATAR_LLM);
            break;
        case Role::TOOL:
            this->avatarFilePath = QString(AVATAR_TOOL);
            break;
        case Role::SYSTEM:
            this->avatarFilePath = QString(AVATAR_SYSTEM);
            break;
        default:
            this->avatarFilePath = QString(AVATAR_UNKNOW);
            break;
        }
    }

    Message(const QString &id, const QString &text, Role role,
            const QString &avatarFilePath,
            const QString &toolCalls, const QString &toolCallId,
            const QString &createdTime)
        : id(id), text(text), role(role), avatarFilePath(avatarFilePath), toolCalls(toolCalls), toolCallId(toolCallId), createdTime(createdTime)
    {
    }

    Message(const QString &text,
            Role role = USER, const QString &avatarFilePath = QString(),
            const QString &toolCalls = QString(), const QString &toolCallId = QString(),
            const QString &createdTime = getCurrentDateTime())
        : id(generateUuid()), text(text), role(role), avatarFilePath(avatarFilePath), toolCalls(toolCalls), toolCallId(toolCallId), createdTime(createdTime)
    {
        if (this->avatarFilePath.isEmpty())
        {
            switch (role)
            {
            case Role::USER:
                this->avatarFilePath = QString(DEFAULT_AVATAR_USER);
                break;
            case Role::ASSISTANT:
                this->avatarFilePath = QString(DEFAULT_AVATAR_LLM);
                break;
            case Role::TOOL:
                this->avatarFilePath = QString(AVATAR_TOOL);
                break;
            case Role::SYSTEM:
                this->avatarFilePath = QString(AVATAR_SYSTEM);
                break;
            default:
                this->avatarFilePath = QString(AVATAR_UNKNOW);
                break;
            }
        }
    }
};
// Q_DECLARE_METATYPE(Message)

/**
 * 获取默认字体
 */
inline QFont getGlobalFont()
{
    QFont font("Microsoft YaHei");
    font.setPointSize(10);
    font.setStyleHint(QFont::SansSerif); // 微软雅黑不存在时优先考虑无衬线字体
    return font;
}

/**
 * 获取默认字体下展示str所需的宽度
 */
inline int getFontWidth(const QString &str)
{
    return QFontMetrics(getGlobalFont()).horizontalAdvance(str);
}

/**
 * 获取默认字体下展示文本的高度
 */
inline int getFontHeight()
{
    return QFontMetrics(getGlobalFont()).height();
}

/**
 * 获取uuid
 */
inline QString generateUuid()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

/**
 * 获取当前时间 yyyy-mm-dd hh:mm:ss
 */
inline QString getCurrentDateTime()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
}
#endif // GLOBAL_H