#include "Material.hpp"
namespace MEngine
{
Material::Material(std::shared_ptr<DescriptorManager> descriptorManager,
                   std::shared_ptr<PipelineManager> pipelineManager,
                   std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager, PipelineType pipelineType,
                   PipelineLayoutType pipelineLayoutType)
    : mDescriptorManager(descriptorManager), mPipelineManager(pipelineManager),
      mPipelineLayoutManager(pipelineLayoutManager), mPipelineType(pipelineType),
      mPipelineLayoutType(pipelineLayoutType)
{
    // 创建描述符集
    auto descriptorSetLayout = mPipelineLayoutManager->GetDescriptorSetLayout(mPipelineLayoutType);
    if (descriptorSetLayout)
    {
        mDescriptorSet = std::move(mDescriptorManager->AllocateUniqueDescriptorSet({descriptorSetLayout})[0]);
    }
}

void Material::AddTexture(std::shared_ptr<Texture> texture)
{
    auto type = texture->GetTextureType();
    mTextures[type] = texture;
    // 更新描述符集
    switch (type)
    {
    case TextureType::Albedo:
        mDescriptorManager->UpdateCombinedSamplerImageDescriptorSet(
            std::vector<ImageDescriptor>{ImageDescriptor{texture->GetImageView(), texture->GetSampler()}}, 0,
            mDescriptorSet.get());
        break;
    case TextureType::BaseColor:
        mDescriptorManager->UpdateCombinedSamplerImageDescriptorSet(
            std::vector<ImageDescriptor>{ImageDescriptor{texture->GetImageView(), texture->GetSampler()}}, 0,
            mDescriptorSet.get());
        break;

        // TODO
    }
}

std::vector<std::shared_ptr<Texture>> Material::GetTextures() const
{
    std::vector<std::shared_ptr<Texture>> textures;
    for (const auto &pair : mTextures)
    {
        textures.push_back(pair.second);
    }
    return textures;
}
std::shared_ptr<Texture> Material::GetTexture(TextureType type) const
{
    auto it = mTextures.find(type);
    if (it != mTextures.end())
    {
        return it->second;
    }
    return nullptr;
}
} // namespace MEngine