#include "Logger.hpp"
#include "gtest/gtest.h"
#include <iostream>
using namespace MEngine;
using namespace std;

TEST(LoggerTest, logger_level)
{
    Logger::Instance().SetLevel(LogLevel::LOG_TRACE);
    EXPECT_EQ(Logger::Instance().mLogger->level(), spdlog::level::trace);

    Logger::Instance().SetLevel(LogLevel::LOG_DEBUG);
    EXPECT_EQ(Logger::Instance().mLogger->level(), spdlog::level::debug);

    Logger::Instance().SetLevel(LogLevel::LOG_INFO);
    EXPECT_EQ(Logger::Instance().mLogger->level(), spdlog::level::info);

    Logger::Instance().SetLevel(LogLevel::LOG_ERROR);
    EXPECT_EQ(Logger::Instance().mLogger->level(), spdlog::level::err);

    Logger::Instance().SetLevel(LogLevel::LOG_FATAL);
    EXPECT_EQ(Logger::Instance().mLogger->level(), spdlog::level::critical);
}
