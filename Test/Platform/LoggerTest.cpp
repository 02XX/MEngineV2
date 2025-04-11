#include "Interface/ILogger.hpp"
#include "SpdLogger.hpp"
#include "gtest/gtest.h"
#include <iostream>
using namespace MEngine;
using namespace std;

class LoggerTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        // Initialize the logger
        mLogger = std::make_shared<SpdLogger>();
    }

    void TearDown() override
    {
        // Clean up if necessary
    }
    std::shared_ptr<SpdLogger> mLogger;
};
TEST_F(LoggerTest, Log)
{
    mLogger->Trace("This is a trace message");
    mLogger->Info("This is an info message");
    mLogger->Debug("This is a debug message");
    mLogger->Warn("This is a warning message");
    mLogger->Error("This is an error message");
    mLogger->Fatal("This is a critical message");
}