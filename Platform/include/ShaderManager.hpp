#pragma once
#include "Context.hpp"
#include "Interface/ILogger.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include <vulkan/vulkan.hpp>

namespace MEngine
{

class ShaderManager final : public NoCopyable
{
  private:
    // DI
    std::shared_ptr<ILogger> mLogger;
    std::shared_ptr<Context> mContext;

  private:
    std::filesystem::path mShaderPath = std::filesystem::current_path() / "Resource" / "Shader";

  public:
    ShaderManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context);

    void LoadShaderModule(std::string name, const std::filesystem::path &path);
    vk::ShaderModule GetShaderModule(std::string name);

  private:
    std::unordered_map<std::string, vk::UniqueShaderModule> mShaderModules;
};

} // namespace MEngine