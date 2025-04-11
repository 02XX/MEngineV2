#pragma once
#include "Interface/IConfigure.hpp"
#include "Interface/ILogger.hpp"
#include <fstream>
#include <iostream>
namespace fs = std::filesystem;
using Json = nlohmann::json;
namespace MEngine
{
class Configure final : public IConfigure
{

  private:
    Json mJson;
    fs::path mPath;

  public:
    Configure();
    void SetJsonSettingFile(const fs::path &path) override;
    const Json &GetJson() const override;
};
} // namespace MEngine