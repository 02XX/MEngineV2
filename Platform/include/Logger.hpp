#pragma once
#include "MEngine.hpp"
#include "spdlog/async.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace MEngine
{
enum class LogLevel
{
    TRACE,
    DEBUG,
    INFO,
    ERROR,
    FATAL,
};
class MENGINE_API Logger
{
  private:
    Logger();
    std::shared_ptr<spdlog::sinks::sink> mConsoleSink;
    std::shared_ptr<spdlog::sinks::sink> mFileSink;

  public:
    ~Logger();
    std::shared_ptr<spdlog::logger> mLogger;

    static Logger &Instance();
    void SetLevel(LogLevel level);

    template <typename... Args> void trace(fmt::format_string<Args...> fmt, Args &&...args)
    {
        mLogger->trace(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args> void debug(fmt::format_string<Args...> fmt, Args &&...args)
    {
        mLogger->debug(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args> void info(fmt::format_string<Args...> fmt, Args &&...args)
    {
        mLogger->info(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args> void warn(fmt::format_string<Args...> fmt, Args &&...args)
    {
        mLogger->warn(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args> void error(fmt::format_string<Args...> fmt, Args &&...args)
    {
        mLogger->error(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args> void critical(fmt::format_string<Args...> fmt, Args &&...args)
    {
        mLogger->critical(fmt, std::forward<Args>(args)...);
    }
};

#define LogT(...) Logger::Instance().trace(__VA_ARGS__)
#define LogD(...) Logger::Instance().debug(__VA_ARGS__)
#define LogI(...) Logger::Instance().info(__VA_ARGS__)
#define LogW(...) Logger::Instance().warn(__VA_ARGS__)
#define LogE(...) Logger::Instance().error(__VA_ARGS__)
#define LogC(...) Logger::Instance().critical(__VA_ARGS__)
} // namespace MEngine
