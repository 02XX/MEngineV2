#include "Configure.hpp"
#include "Interface/IConfigure.hpp"
#include "Interface/ILogger.hpp"

#include "SpdLogger.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include <string>

using namespace MEngine;

class ConfigureTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        // Initialize the logger
        mConfigure = std::make_shared<Configure>();
        mLogger = std::make_shared<SpdLogger>(mConfigure);
    }

    void TearDown() override
    {
        // Clean up if necessary
    }
    std::shared_ptr<SpdLogger> mLogger;
    std::shared_ptr<IConfigure> mConfigure;
};

TEST_F(ConfigureTest, LoadJsonFile)
{
    std::filesystem::path jsonPath = std::filesystem::current_path() / "appsettings.json";
    EXPECT_NO_THROW(mConfigure->SetJsonSettingFile(jsonPath););
}
TEST_F(ConfigureTest, GetValue)
{
    auto value = mConfigure->GetJson()["Logging"]["LogLevel"]["Debug"].get<std::string>();
    mLogger->Info("Default log level: {}", value);
    EXPECT_EQ(value, "Debug");
}