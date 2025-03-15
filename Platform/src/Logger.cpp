#include "Logger.hpp"

namespace MEngine
{
Logger::Logger()
{
    try
    {
        spdlog::init_thread_pool(8192, 1);
        if (!spdlog::thread_pool())
        {
            std::cerr << "Failed to initialize thread pool!" << std::endl;
            return;
        }
        // console sink
        mConsoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        // file sink
        std::filesystem::path logPath = std::filesystem::current_path() / "MEngine.log";
        mFileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("./MEngine.log", 5 * 1024 * 1024, 3);
        mLogger =
            std::make_shared<spdlog::async_logger>("MEngineLogger", spdlog::sinks_init_list{mConsoleSink, mFileSink},
                                                   spdlog::thread_pool(), spdlog::async_overflow_policy::block);
        mLogger->set_level(spdlog::level::trace);
        mLogger->set_pattern("%^[%Y-%m-%d %H:%M:%S.%e] [%20s:%-4#] %l - %v%$");
        spdlog::register_logger(mLogger);
        spdlog::set_default_logger(mLogger);
    }
    catch (const spdlog::spdlog_ex &e)
    {
        std::cerr << "Logger initialization failed: " << e.what() << std::endl;
    }
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

    case LogLevel::TRACE:
        mLogger->set_level(spdlog::level::trace);
        break;
    case LogLevel::DEBUG:
        mLogger->set_level(spdlog::level::debug);
        break;
    case LogLevel::INFO:
        mLogger->set_level(spdlog::level::info);
        break;
    case LogLevel::ERROR:
        mLogger->set_level(spdlog::level::err);
        break;
    case LogLevel::FATAL:
        mLogger->set_level(spdlog::level::critical);
        break;
    }
}
void Logger::SetFileLogPath(const std::string &path, size_t maxFileSize, size_t maxFiles)
{
    mFileSink.reset();
    mLogger.reset();
    mFileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(path, maxFileSize, maxFiles);
    if (mLogger)
    {
        mLogger->sinks().clear();
        mLogger->sinks().emplace_back(mConsoleSink);
        mLogger->sinks().emplace_back(mFileSink);
    }
}
} // namespace MEngine
