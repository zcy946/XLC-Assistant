#ifndef GLOBAL_H
#define GLOBAL_H

#include <QFont>
#include <QFontMetrics>
#include <QApplication>
#include <QVector>
#include <QVariant>
#include <QDateTime>
#include <QUuid>

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

struct McpServer
{
    enum Type
    {
        stdio = 0,
        sse = 1,
        streambleHttp = 2
    };
    QString uuid;
    QString name;
    QString description;
    Type type;
    int timeout; // 单位: 秒(s)
    // stdio参数
    QString command;
    QVector<QString> args;
    QMap<QString, QString> envVars;
    // sse&streambleHttp参数
    QString url;
    QString requestHeaders;

    McpServer()
        : uuid(generateUuid()),
          name(),
          description(),
          type(sse),
          timeout(60),
          command(),
          args(),
          envVars(),
          url(),
          requestHeaders()
    {
    }
    McpServer(const QString &name,
              const QString &description,
              Type type,
              int timeout,
              const QString &command,
              const QVector<QString> &args,
              const QMap<QString, QString> &envVars,
              const QString &url,
              const QString &requestHeaders)
        : uuid(generateUuid()),
          name(name),
          description(description),
          type(type),
          timeout(timeout),
          command(command),
          args(args),
          envVars(envVars),
          url(url),
          requestHeaders(requestHeaders)
    {
    }
};
Q_DECLARE_METATYPE(McpServer)

struct Agent
{
    QString uuid;
    QString name;
    QString description;
    int children; // 以该agent为模板的对话数量
    // llm参数
    int context;
    QString systemPrompt;
    QString modelName;
    double temperature;
    double topP;
    int maxTokens;
    QVector<McpServer> mcpServers;

    Agent()
        : name(),
          description(),
          children(),
          context(),
          systemPrompt(),
          modelName(),
          temperature(),
          topP(),
          maxTokens(),
          mcpServers()
    {
    }
    Agent(const QString &name,
          const QString &description,
          int children,
          int context,
          const QString &systemPrompt,
          const QString &modelName,
          double temperature,
          double topP,
          int maxTokens,
          const QVector<McpServer> &mcpServers = QVector<McpServer>())
        : uuid(generateUuid()),
          name(name),
          description(description),
          children(children),
          context(context),
          systemPrompt(systemPrompt),
          modelName(modelName),
          temperature(temperature),
          topP(topP),
          maxTokens(maxTokens),
          mcpServers(mcpServers)
    {
    }
};
Q_DECLARE_METATYPE(Agent)

struct Conversation
{
    QString uuid;
    QString summary; // 对话摘要
    QDateTime createdTime;
    QDateTime updatedTime;

    Conversation()
        : uuid(generateUuid()),
          summary(),
          createdTime(QDateTime::currentDateTime()),
          updatedTime(QDateTime::currentDateTime())
    {
    }
    Conversation(const QString &summary,
                 const QDateTime &createdTime,
                 const QDateTime &updatedTime)
        : uuid(generateUuid()),
          summary(summary),
          createdTime(createdTime),
          updatedTime(updatedTime)
    {
    }
};
Q_DECLARE_METATYPE(Conversation)

inline void registerAllMetaType()
{
    qRegisterMetaType<McpServer>("McpServer");
    qRegisterMetaType<Agent>("Agent");
    qRegisterMetaType<Conversation>("Conversation");
}

#endif // GLOBAL_H