#pragma once
#include "fmt/format.h"
#include <source_location>
#include <string>
namespace MEngine
{
class ILogger
{
  public:
    virtual ~ILogger() = default;
    template <typename... Args> void Trace(fmt::format_string<Args...> format, Args &&...args)
    {
        std::string message = fmt::format(format, std::forward<Args>(args)...);
        LogTrace(message);
    }
    template <typename... Args> void Debug(fmt::format_string<Args...> format, Args &&...args)
    {
        std::string message = fmt::format(format, std::forward<Args>(args)...);
        LogDebug(message);
    }
    template <typename... Args> void Info(fmt::format_string<Args...> format, Args &&...args)
    {
        std::string message = fmt::format(format, std::forward<Args>(args)...);
        LogInfo(message);
    }
    template <typename... Args> void Error(fmt::format_string<Args...> format, Args &&...args)
    {
        std::string message = fmt::format(format, std::forward<Args>(args)...);
        LogError(message);
    }
    template <typename... Args> void Fatal(fmt::format_string<Args...> format, Args &&...args)
    {
        std::string message = fmt::format(format, std::forward<Args>(args)...);
        LogFatal(message);
    }

  private:
    virtual void LogTrace(const std::string &message,
                          const std::source_location &loc = std::source_location::current()) = 0;
    virtual void LogDebug(const std::string &message,
                          const std::source_location &loc = std::source_location::current()) = 0;
    virtual void LogInfo(const std::string &message,
                         const std::source_location &loc = std::source_location::current()) = 0;
    virtual void LogError(const std::string &message,
                          const std::source_location &loc = std::source_location::current()) = 0;
    virtual void LogFatal(const std::string &message,
                          const std::source_location &loc = std::source_location::current()) = 0;
};
} // namespace MEngine