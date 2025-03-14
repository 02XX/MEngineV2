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
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        // file sink
        auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("MEngine.log", 5 * 1024 * 1024, 3);
        mLogger =
            std::make_shared<spdlog::async_logger>("MEngineLogger", spdlog::sinks_init_list{consoleSink, fileSink},
                                                   spdlog::thread_pool(), spdlog::async_overflow_policy::block);
        mLogger->set_level(spdlog::level::trace);
        mLogger->set_pattern("%^[%Y-%m-%d %H:%M:%S.%e] [%20s:%-4#] %l - %v%$");
        spdlog::register_logger(mLogger);
        spdlog::set_default_logger(mLogger);
    }
    catch (const spdlog::spdlog_ex &e)
    {
        std::cerr << "Logger initialization failed." << std::endl;
    }
}

Logger &Logger::Instance()
{
    static Logger logger;
    return logger;
}
} // namespace MEngine
