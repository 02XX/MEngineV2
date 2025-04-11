#include "Configure.hpp"

namespace MEngine
{
Configure::Configure()
{
    mPath = fs::current_path() / "appsettings.json";
    SetJsonSettingFile(mPath);
}
void Configure::SetJsonSettingFile(const fs::path &path)
{
    mPath = path;
    std::cout << "Loading configuration file: " << mPath.string() << std::endl;
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Failed to open configuration file: " << path.string() << std::endl;
        throw std::runtime_error("Failed to open configuration file: " + path.string());
    }
    try
    {
        mJson = Json::parse(file);
        std::cout << "Configuration file loaded successfully." << std::endl;
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