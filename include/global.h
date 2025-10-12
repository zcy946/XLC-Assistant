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

static const char *FILE_CONFIG = "./config.ini";

static const char *AVATAR_UNKNOW = "://image/avatar_unknow.png";
static const char *AVATAR_TOOL = "://image/avatar_tool.png";
static const char *AVATAR_SYSTEM = "://image/avatar_system.png";
static const char *DEFAULT_AVATAR_USER = "://image/default_avatar_user.png";
static const char *DEFAULT_AVATAR_LLM = "://image/default_avatar_llm.png";

static const char *DEFAULT_CONTENT_CLEAR_CONTEXT = "XLC_ASSISTANT_CLEAR_CONTEXT"; // 清除上下文的Role为SYSTEM的Message的Content

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
 * 获取当前时间 yyyy-MM-dd HH:mm:ss
 */
inline QString getCurrentDateTime()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
}

#endif // GLOBAL_H