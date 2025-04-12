#include "MaterialManager.hpp"

namespace MEngine
{
MaterialManager::MaterialManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                                 std::shared_ptr<PipelineManager> pipelineManager,
                                 std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager,
                                 std::shared_ptr<DescriptorManager> descriptorManager,
                                 std::shared_ptr<SamplerManager> samplerManager,
                                 std::shared_ptr<TextureManager> textureManager)
    : mLogger(logger), mContext(context), mPipelineManager(pipelineManager),
      mPipelineLayoutManager(pipelineLayoutManager), mDescriptorManager(descriptorManager),
      mSamplerManager(samplerManager), mTextureManager(textureManager)
{
}
std::shared_ptr<Material> MaterialManager::CreateTranslucencyMaterial(std::filesystem::path baseColorPath)
{
    auto material = std::make_shared<Material>(mDescriptorManager, mPipelineManager, mPipelineLayoutManager,
                                               PipelineType::Translucency, PipelineLayoutType::TranslucencyLayout);
    auto texture = mTextureManager->CreateTexture(baseColorPath, TextureType::BaseColor);
    material->AddTexture(texture);
    auto id = std::hash<std::string>{}(baseColorPath.string());
    mMaterials[id] = material;
    return material;
}
std::shared_ptr<Material> MaterialManager::GetMaterial(uint32_t id)
{
    auto it = mMaterials.find(id);
    if (it != mMaterials.end())
    {
        auto material = it->second.lock();
        if (material)
        {
            return material;
        }
        else
        {
            mMaterials.erase(it);
        }
    }
    return nullptr;
}
std::shared_ptr<Material> MaterialManager::GetDefaultMaterial()
{
    // TODO: 改成PBR默认材质
    auto material = std::make_shared<Material>(mDescriptorManager, mPipelineManager, mPipelineLayoutManager,
                                               PipelineType::Translucency, PipelineLayoutType::TranslucencyLayout);
    auto texture = mTextureManager->CreateTexture(mDefaultAlbedoPath, TextureType::BaseColor);
    material->AddTexture(texture);
    auto id = std::hash<std::string>{}(mDefaultAlbedoPath.string());
    mMaterials[id] = material;
    return material;
}

} // namespace MEngine