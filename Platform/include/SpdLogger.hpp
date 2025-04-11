#pragma once
#include "Interface/ILogger.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <source_location>
namespace MEngine
{
class SpdLogger final : public ILogger, NoCopyable
{
  private:
    std::shared_ptr<spdlog::sinks::sink> mConsoleSink;
    std::shared_ptr<spdlog::sinks::sink> mFileSink;
    std::shared_ptr<spdlog::logger> mLogger;

    void LogTrace(const std::string &message,
                  const std::source_location &loc = std::source_location::current()) override;
    void LogDebug(const std::string &message,
                  const std::source_location &loc = std::source_location::current()) override;
    void LogWarn(const std::string &message,
                 const std::source_location &loc = std::source_location::current()) override;
    void LogInfo(const std::string &message,
                 const std::source_location &loc = std::source_location::current()) override;
    void LogError(const std::string &message,
                  const std::source_location &loc = std::source_location::current()) override;
    void LogFatal(const std::string &message,
                  const std::source_location &loc = std::source_location::current()) override;

  public:
    SpdLogger();
};
} // namespace MEngine
