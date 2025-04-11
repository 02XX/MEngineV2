#pragma once
#include "Interface/IConfigure.hpp"
#include "Interface/ILogger.hpp"
#include <fstream>
namespace fs = std::filesystem;
using Json = nlohmann::json;
namespace MEngine
{
class Configure final : public IConfigure
{
  private:
    // DI
    std::shared_ptr<ILogger> mLogger;

  private:
    Json mJson;
    fs::path mPath;

  public:
    Configure(std::shared_ptr<ILogger> logger);
    void SetJsonSettingFile(const fs::path &path) override;
    const Json &GetJson() const override;
};
} // namespace MEngine