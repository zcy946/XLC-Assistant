#ifndef LOGGER_H
#define LOGGER_H

// 必须在包含 spdlog.h 之前定义这个宏，或者通过编译器选项-D来定义
// 比如在CMake中：target_compile_definitions(target PRIVATE SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
// 如果不定义，spdlog会默认一个较高级别（如INFO），导致TRACE/DEBUG日志和位置信息被编译时优化掉。

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>

inline void initLogger()
{
    // 使用静态变量防止重复初始化
    static bool initialized = false;
    if (initialized)
    {
        return;
    }
    initialized = true;

    // 1. 创建 sinks
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    // 使用 %@ 或 %s:%# 来显示文件名和行号
    console_sink->set_pattern("%^[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v%$");
    console_sink->set_level(spdlog::level::trace);

    size_t max_size = 1024 * 1024 * 5; // 5 MB
    size_t max_files = 3;              // 保留最多3个备份文件 (logs.txt, logs.1.txt, logs.2.txt)
    auto rotating_file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs.txt", max_size, max_files);
    // 使用 %@ 或 %s:%# 来显示文件名和行号
    rotating_file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v");
    rotating_file_sink->set_level(spdlog::level::info);

    // 2. 创建 logger 并关联 sinks
    auto logger = std::make_shared<spdlog::logger>("multi_sink", spdlog::sinks_init_list{console_sink, rotating_file_sink});
    logger->set_level(spdlog::level::trace);

    // 3. 注册并设为默认 logger
    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
}

#define LOG_TRACE(...) SPDLOG_TRACE(__VA_ARGS__)
#define LOG_DEBUG(...) SPDLOG_DEBUG(__VA_ARGS__)
#define LOG_INFO(...) SPDLOG_INFO(__VA_ARGS__)
#define LOG_WARN(...) SPDLOG_WARN(__VA_ARGS__)
#define LOG_ERROR(...) SPDLOG_ERROR(__VA_ARGS__)
#define LOG_CRITICAL(...) SPDLOG_CRITICAL(__VA_ARGS__)

#endif