#pragma once
#include "nlohmann/json.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using Json = nlohmann::json;
namespace MEngine
{
class IConfigure
{
  public:
    virtual ~IConfigure() = default;
    virtual void SetJsonSettingFile(const fs::path &path) = 0;
    virtual const Json &GetJson() const = 0;
};
} // namespace MEngine
