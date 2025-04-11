#include "Configure.hpp"

namespace MEngine
{
Configure::Configure(std::shared_ptr<ILogger> logger) : mLogger(logger)
{
    mPath = fs::current_path() / "appsettings.json";
    SetJsonSettingFile(mPath);
}
void Configure::SetJsonSettingFile(const fs::path &path)
{
    mPath = path;
    mLogger->Info("Loading configuration file: {}", path.string());
    std::ifstream file(path);
    if (!file.is_open())
    {
        mLogger->Error("Failed to open configuration file: {}", path.string());
        throw std::runtime_error("Failed to open configuration file: " + path.string());
    }
    try
    {
        mJson = Json::parse(file);
        mLogger->Info("Configuration file loaded successfully: {}", path.string());
    }
    catch (const Json::exception &e)
    {
        throw std::runtime_error("JSON parsing failed: " + std::string(e.what()));
    }
}

const Json &Configure::GetJson() const
{
    if (mJson.empty())
    {
        throw std::runtime_error("Configuration file is not loaded or empty.");
    }
    return mJson;
}
} // namespace MEngine