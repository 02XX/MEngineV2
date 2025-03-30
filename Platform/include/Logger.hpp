#pragma once
#include "MEngine.hpp"
#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace MEngine
{
enum class LogLevel
{
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_ERROR,
    LOG_FATAL,
};
class Logger
{
  private:
    Logger();
    std::shared_ptr<spdlog::sinks::sink> mConsoleSink;
    std::shared_ptr<spdlog::sinks::sink> mFileSink;

  public:
    ~Logger();
    std::shared_ptr<spdlog::logger> mLogger;
    void Init();
    static Logger &Instance();
    void SetLevel(LogLevel level);

    template <typename... Args> void trace(const char *file, int line, fmt::format_string<Args...> fmt, Args &&...args)
    {
        mLogger->log(spdlog::source_loc{file, line, SPDLOG_FUNCTION}, spdlog::level::trace, fmt,
                     std::forward<Args>(args)...);
    }

    template <typename... Args> void debug(const char *file, int line, fmt::format_string<Args...> fmt, Args &&...args)
    {
        mLogger->log(spdlog::source_loc{file, line, SPDLOG_FUNCTION}, spdlog::level::debug, fmt,
                     std::forward<Args>(args)...);
    }

    template <typename... Args> void info(const char *file, int line, fmt::format_string<Args...> fmt, Args &&...args)
    {
        mLogger->log(spdlog::source_loc{file, line, SPDLOG_FUNCTION}, spdlog::level::info, fmt,
                     std::forward<Args>(args)...);
    }

    template <typename... Args> void warn(const char *file, int line, fmt::format_string<Args...> fmt, Args &&...args)
    {
        mLogger->log(spdlog::source_loc{file, line, SPDLOG_FUNCTION}, spdlog::level::warn, fmt,
                     std::forward<Args>(args)...);
    }

    template <typename... Args> void error(const char *file, int line, fmt::format_string<Args...> fmt, Args &&...args)
    {
        mLogger->log(spdlog::source_loc{file, line, SPDLOG_FUNCTION}, spdlog::level::err, fmt,
                     std::forward<Args>(args)...);
    }

    template <typename... Args>
    void critical(const char *file, int line, fmt::format_string<Args...> fmt, Args &&...args)
    {
        mLogger->log(spdlog::source_loc{file, line, SPDLOG_FUNCTION}, spdlog::level::critical, fmt,
                     std::forward<Args>(args)...);
    }
};

#define LogT(...) Logger::Instance().trace(__FILE__, __LINE__, __VA_ARGS__)
#define LogD(...) Logger::Instance().debug(__FILE__, __LINE__, __VA_ARGS__)
#define LogI(...) Logger::Instance().info(__FILE__, __LINE__, __VA_ARGS__)
#define LogW(...) Logger::Instance().warn(__FILE__, __LINE__, __VA_ARGS__)
#define LogE(...) Logger::Instance().error(__FILE__, __LINE__, __VA_ARGS__)
#define LogC(...) Logger::Instance().critical(__FILE__, __LINE__, __VA_ARGS__)
} // namespace MEngine
