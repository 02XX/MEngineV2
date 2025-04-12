#include "ShaderManager.hpp"

namespace MEngine
{
ShaderManager::ShaderManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context)
    : mLogger(logger), mContext(context)
{
    // LoadShaderModule("defferGBuffer", "assets/shaders/defferGBuffer.spv");
    // LoadShaderModule("defferLighting", "assets/shaders/defferLighting.spv");
    // LoadShaderModule("shadowDepth", "assets/shaders/shadowDepth.spv");
    // LoadShaderModule("translucency", "assets/shaders/translucency.spv");
    // LoadShaderModule("postProcess", "assets/shaders/postProcess.spv");
    // LoadShaderModule("sky", "assets/shaders/sky.spv");
    // LoadShaderModule("ui", "assets/shaders/ui.spv");
}
void ShaderManager::LoadShaderModule(std::string name, const std::filesystem::path &path)
{
    std::filesystem::path shaderPath = mShaderPath / path;
    std::ifstream file(shaderPath.string().c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        mLogger->Error("Failed to open file: {}", shaderPath.string());
        throw std::runtime_error("Failed to open file");
    }
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0, std::ios::beg);
    file.read((char *)buffer.data(), fileSize);
    file.close();
    vk::ShaderModuleCreateInfo shaderModuleCreateInfo{};
    shaderModuleCreateInfo.setCodeSize(buffer.size()).setPCode(reinterpret_cast<const uint32_t *>(buffer.data()));
    auto shaderModule = mContext->GetDevice().createShaderModuleUnique(shaderModuleCreateInfo);
    if (!shaderModule)
    {
        mLogger->Error("Failed to create shader module: {}", shaderPath.string());
        throw std::runtime_error("Failed to create shader module");
    }
    mShaderModules.emplace(std::move(name), std::move(shaderModule));
    mLogger->Debug("Shader Module loaded: {}", shaderPath.string());
}
vk::ShaderModule ShaderManager::GetShaderModule(std::string name)
{
    if (mShaderModules.find(name) == mShaderModules.end())
    {
        mLogger->Error("Shader Module not found.");
        return nullptr;
    }
    return mShaderModules[name].get();
}
} // namespace MEngine