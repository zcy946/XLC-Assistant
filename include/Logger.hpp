#ifndef LOGGER_H
#define LOGGER_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <QString>
#include <memory>
#include <string>

constexpr const char * FILE_NAME = "logs/logs.log";

#define XLC_LOG_TRACE(...) Logger::trace({__FILE__, __LINE__, SPDLOG_FUNCTION}, __VA_ARGS__)
#define XLC_LOG_DEBUG(...) Logger::debug({__FILE__, __LINE__, SPDLOG_FUNCTION}, __VA_ARGS__)
#define XLC_LOG_INFO(...) Logger::info({__FILE__, __LINE__, SPDLOG_FUNCTION}, __VA_ARGS__)
#define XLC_LOG_WARN(...) Logger::warn({__FILE__, __LINE__, SPDLOG_FUNCTION}, __VA_ARGS__)
#define XLC_LOG_ERROR(...) Logger::error({__FILE__, __LINE__, SPDLOG_FUNCTION}, __VA_ARGS__)
#define XLC_LOG_CRITICAL(...) Logger::critical({__FILE__, __LINE__, SPDLOG_FUNCTION}, __VA_ARGS__)

class Logger
{
public:
    static void init()
    {
        static bool initialized = false;
        if (initialized)
            return;
        initialized = true;

        // 控制台输出
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_pattern("%^[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v%$");
        console_sink->set_level(spdlog::level::trace);

        // 文件输出（可旋转）
        auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(FILE_NAME, 1024 * 1024 * 5, 3);
        rotating_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v");
#ifdef QT_DEBUG
        rotating_sink->set_level(spdlog::level::trace);
#else
        rotating_sink->set_level(spdlog::level::warn);
#endif

        auto logger = std::make_shared<spdlog::logger>("multi_sink", spdlog::sinks_init_list{console_sink, rotating_sink});
        logger->set_level(spdlog::level::trace);
        spdlog::register_logger(logger);
        spdlog::set_default_logger(logger);
    }

    // QString 转换支持
    template <typename T>
    static auto convert(const T &value) -> decltype(value)
    {
        return value;
    }

    static std::string convert(const QString &qstr)
    {
        return qstr.toStdString(); // 或 qstr.toUtf8().constData() 保证 UTF-8
    }

    // 自动格式化 + 源位置日志调用封装
    template <typename... Args>
    static void trace(spdlog::source_loc loc, const char *fmt, Args &&...args)
    {
        spdlog::log(loc, spdlog::level::trace, fmt, convert(std::forward<Args>(args))...);
    }

    template <typename... Args>
    static void debug(spdlog::source_loc loc, const char *fmt, Args &&...args)
    {
        spdlog::log(loc, spdlog::level::debug, fmt, convert(std::forward<Args>(args))...);
    }

    template <typename... Args>
    static void info(spdlog::source_loc loc, const char *fmt, Args &&...args)
    {
        spdlog::log(loc, spdlog::level::info, fmt, convert(std::forward<Args>(args))...);
    }

    template <typename... Args>
    static void warn(spdlog::source_loc loc, const char *fmt, Args &&...args)
    {
        spdlog::log(loc, spdlog::level::warn, fmt, convert(std::forward<Args>(args))...);
    }

    template <typename... Args>
    static void error(spdlog::source_loc loc, const char *fmt, Args &&...args)
    {
        spdlog::log(loc, spdlog::level::err, fmt, convert(std::forward<Args>(args))...);
    }

    template <typename... Args>
    static void critical(spdlog::source_loc loc, const char *fmt, Args &&...args)
    {
        spdlog::log(loc, spdlog::level::critical, fmt, convert(std::forward<Args>(args))...);
    }
};

#endif // LOGGER_H