#pragma once
#include "MEngine.hpp"
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
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
    std::string mFileLogPath;
    std::shared_ptr<spdlog::sinks::sink> mConsoleSink;
    std::shared_ptr<spdlog::sinks::sink> mFileSink;

  public:
    std::shared_ptr<spdlog::logger> mLogger;
    static Logger &Instance();
    void SetLevel(LogLevel level);
    void SetFileLogPath(const std::string &path, size_t maxFileSize = 5 * 1024 * 1024, size_t maxFiles = 3);
};

#define LogT(...) SPDLOG_LOGGER_TRACE(Logger::Instance().mLogger, ##__VA_ARGS__)
#define LogD(...) SPDLOG_LOGGER_DEBUG(Logger::Instance().mLogger, ##__VA_ARGS__)
#define LogI(...) SPDLOG_LOGGER_INFO(Logger::Instance().mLogger, ##__VA_ARGS__)
#define LogW(...) SPDLOG_LOGGER_WARN(Logger::Instance().mLogger, ##__VA_ARGS__)
#define LogE(...) SPDLOG_LOGGER_ERROR(Logger::Instance().mLogger, ##__VA_ARGS__)
#define LogC(...) SPDLOG_LOGGER_CRITICAL(Logger::Instance().mLogger, ##__VA_ARGS__)
} // namespace MEngine
