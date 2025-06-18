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

constexpr const char *FILE_MCPSERVERS = "./config/MCPServers.json";
constexpr const char *FILE_AGENTS = "./config/Agents.json";
constexpr const char *BASE_URL = "https://api.deepseek.com";
constexpr const char *ENDPOINT = "/v1/chat/completions";
constexpr const char *API_KEY = "sk-67827bd147dc43afbb9a982349c4be31";

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

#endif // GLOBAL_H