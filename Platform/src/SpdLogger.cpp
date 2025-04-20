#include "SpdLogger.hpp"

namespace MEngine
{
void SpdLogger::LogTrace(const std::string &message, const std::source_location &loc)
{
    mLogger->log(spdlog::source_loc{loc.file_name(), static_cast<std::uint_least32_t>(loc.line()), loc.function_name()},
                 spdlog::level::trace, message);
}
void SpdLogger::LogDebug(const std::string &message, const std::source_location &loc)
{
    mLogger->log(spdlog::source_loc{loc.file_name(), static_cast<std::uint_least32_t>(loc.line()), loc.function_name()},
                 spdlog::level::debug, message);
}
void SpdLogger::LogWarn(const std::string &message, const std::source_location &loc)
{
    mLogger->log(spdlog::source_loc{loc.file_name(), static_cast<std::uint_least32_t>(loc.line()), loc.function_name()},
                 spdlog::level::warn, message);
}
void SpdLogger::LogInfo(const std::string &message, const std::source_location &loc)
{
    mLogger->log(spdlog::source_loc{loc.file_name(), static_cast<std::uint_least32_t>(loc.line()), loc.function_name()},
                 spdlog::level::info, message);
}
void SpdLogger::LogError(const std::string &message, const std::source_location &loc)
{
    mLogger->log(spdlog::source_loc{loc.file_name(), static_cast<std::uint_least32_t>(loc.line()), loc.function_name()},
                 spdlog::level::err, message);
}
void SpdLogger::LogFatal(const std::string &message, const std::source_location &loc)
{
    mLogger->log(spdlog::source_loc{loc.file_name(), static_cast<std::uint_least32_t>(loc.line()), loc.function_name()},
                 spdlog::level::critical, message);
}
SpdLogger::SpdLogger(std::shared_ptr<IConfigure> configure) : mConfigure(configure)
{
    try
    {
        // console sink
        mConsoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        // file sink
        std::filesystem::path logPath = std::filesystem::current_path() / "MEngine.log";
        mFileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logPath.string(), 5 * 1024 * 1024, 3);
        mLogger = std::make_shared<spdlog::logger>("MEngineLogger", spdlog::sinks_init_list{mConsoleSink, mFileSink});
        auto level = mConfigure->GetJson()["Logger"]["Level"].get<std::string>();
        if (level == "trace")
        {
            mLogger->set_level(spdlog::level::trace);
        }
        else if (level == "debug")
        {
            mLogger->set_level(spdlog::level::debug);
        }
        else if (level == "info")
        {
            mLogger->set_level(spdlog::level::info);
        }
        else if (level == "warn")
        {
            mLogger->set_level(spdlog::level::warn);
        }
        else if (level == "error")
        {
            mLogger->set_level(spdlog::level::err);
        }
        else if (level == "critical")
        {
            mLogger->set_level(spdlog::level::critical);
        }
        else
        {
            mLogger->error("Invalid log level: {}", level);
        }
        auto formatter = std::make_unique<spdlog::pattern_formatter>();
        formatter->add_flag<UppercaseLevelFormatter>('U');
        formatter->set_pattern("%^[%Y-%m-%d %H:%M:%S.%e] [%20s:%-4#] %U: %v%$");
        mLogger->set_formatter(std::move(formatter));
    }
    catch (const spdlog::spdlog_ex &e)
    {
        std::cerr << "Logger initialization failed: " << e.what() << std::endl;
    }
}
} // namespace MEngine
