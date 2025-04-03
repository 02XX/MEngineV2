#include "ShaderManager.hpp"


namespace MEngine
{
void ShaderManager::LoadShaderModule(std::string name, const std::string &path)
{
    auto &context = Context::Instance();
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        LogE("Failed to open file: {}", path);
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
    auto shaderModule = context.GetDevice().createShaderModuleUnique(shaderModuleCreateInfo);
    mShaderModules.emplace(std::move(name), std::move(shaderModule));
    LogD("Shader Module Created.");
}
vk::ShaderModule ShaderManager::GetShaderModule(std::string name)
{
    if (mShaderModules.find(name) == mShaderModules.end())
    {
        LogE("Shader Module not found.");
        return nullptr;
    }
    return mShaderModules[name].get();
}
} // namespace MEngine