#pragma once
#include "Context.hpp"
#include "Logger.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include <vulkan/vulkan.hpp>

namespace MEngine
{

class ShaderManager final : public NoCopyable
{
  public:
    ShaderManager() = default;

    void LoadShaderModule(std::string name, const std::string &path);
    vk::ShaderModule GetShaderModule(std::string name);

  private:
    std::unordered_map<std::string, vk::UniqueShaderModule> mShaderModules;
};

} // namespace MEngine