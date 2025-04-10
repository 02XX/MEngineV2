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

void Material::AddTexture(TextureType type, std::shared_ptr<Texture> texture)
{
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

// std::shared_ptr<Texture> Material::GetTexture(TextureType type)
// {
//     auto it = mTextures.find(type);
//     if (it != mTextures.end())
//     {
//         return it->second;
//     }
//     return nullptr;
// }
} // namespace MEngine