#pragma once
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "MEngine.hpp"
#include <spdlog/async.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace MEngine
{
class MENGINE_API Logger
{
  private:
    Logger();

  public:
    std::shared_ptr<spdlog::logger> mLogger;
    static Logger &Instance();
};

#define LOG_TRACE(...) SPDLOG_LOGGER_TRACE(Logger::Instance().mLogger, ##__VA_ARGS__)
#define LOG_DEBUG(...) SPDLOG_LOGGER_DEBUG(Logger::Instance().mLogger, ##__VA_ARGS__)
#define LOG_INFO(...) SPDLOG_LOGGER_INFO(Logger::Instance().mLogger, ##__VA_ARGS__)
#define LOG_WARN(...) SPDLOG_LOGGER_WARN(Logger::Instance().mLogger, ##__VA_ARGS__)
#define LOG_ERROR(...) SPDLOG_LOGGER_ERROR(Logger::Instance().mLogger, ##__VA_ARGS__)
#define LOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(Logger::Instance().mLogger, ##__VA_ARGS__)
} // namespace MEngine
