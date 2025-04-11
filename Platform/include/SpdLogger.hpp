#pragma once
#include "Interface/IConfigure.hpp"
#include "Interface/ILogger.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include "magic_enum/magic_enum.hpp"
#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <memory>
#include <source_location>

namespace MEngine
{
class SpdLogger final : public ILogger, NoCopyable
{
  private:
    // DI
    std::shared_ptr<IConfigure> mConfigure;

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
    SpdLogger(std::shared_ptr<IConfigure> configure);
};
} // namespace MEngine
