#include "Logger.hpp"

namespace MEngine
{
void Logger::Init()
{
    try
    {
        // console sink
        mConsoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        // file sink
        std::filesystem::path logPath = std::filesystem::current_path() / "MEngine.log";
        mFileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logPath.string(), 5 * 1024 * 1024, 3);
        mLogger = std::make_shared<spdlog::logger>("MEngineLogger", spdlog::sinks_init_list{mConsoleSink, mFileSink});
        mLogger->set_level(spdlog::level::info);
        mLogger->set_pattern("%^[%Y-%m-%d %H:%M:%S.%e] [%20s:%-4#] %l - %v%$");
        spdlog::register_logger(mLogger);
        spdlog::set_default_logger(mLogger);
    }
    catch (const spdlog::spdlog_ex &e)
    {
        std::cerr << "Logger initialization failed: " << e.what() << std::endl;
    }
    std::cout << "Logger initialized" << std::endl;
}
Logger::Logger()
{
}
Logger::~Logger()
{
}
Logger &Logger::Instance()
{
    static Logger logger;
    return logger;
}
void Logger::SetLevel(LogLevel level)
{
    switch (level)
    {
    case LogLevel::LOG_TRACE:
        mLogger->set_level(spdlog::level::trace);
        break;
    case LogLevel::LOG_DEBUG:
        mLogger->set_level(spdlog::level::debug);
        break;
    case LogLevel::LOG_INFO:
        mLogger->set_level(spdlog::level::info);
        break;
    case LogLevel::LOG_ERROR:
        mLogger->set_level(spdlog::level::err);
        break;
    case LogLevel::LOG_FATAL:
        mLogger->set_level(spdlog::level::critical);
        break;
    }
}
} // namespace MEngine
