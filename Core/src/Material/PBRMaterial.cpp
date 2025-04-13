#include "Material/PBRMaterial.hpp"

namespace MEngine
{
PBRMaterial::PBRMaterial(std::shared_ptr<Context> context, std::shared_ptr<TextureManager> textureManager,
                         std::shared_ptr<BufferFactory> bufferFactory,
                         std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager,
                         std::shared_ptr<DescriptorManager> descriptorManager)
    : mContext(context), mTextureManager(textureManager), mBufferFactory(bufferFactory),
      mPipelineLayoutManager(pipelineLayoutManager), mDescriptorManager(descriptorManager)
{
    mMaterialParamsUBO = mBufferFactory->CreateBuffer(BufferType::Uniform, sizeof(PBRMaterialParams));
    auto pbrDescriptorLayout = mPipelineLayoutManager->GetPBRDescriptorSetLayout();
    mMaterialDescriptorSet = std::move(mDescriptorManager->AllocateUniqueDescriptorSet({pbrDescriptorLayout})[0]);

    // 占位纹理
    mMaterialTextures.albedoMap = mTextureManager->GetDefaultTexture();
    mMaterialTextures.normalMap = mTextureManager->GetDefaultTexture();
    mMaterialTextures.metallicRoughnessMap = mTextureManager->GetDefaultTexture();
    mMaterialTextures.aoMap = mTextureManager->GetDefaultTexture();
    mMaterialTextures.emissiveMap = mTextureManager->GetDefaultTexture();
}
PipelineType PBRMaterial::GetPipelineType() const
{
    return mPipelineType;
}
vk::DescriptorSet PBRMaterial::GetMaterialDescriptorSet() const
{
    return mMaterialDescriptorSet.get();
}
void PBRMaterial::SetPBRParameters(const PBRParameters &params)
{
    mMaterialParams.parameters = params;
}
void PBRMaterial::SetPBRTextures(const PBRMaterialTextures &textures)
{
    if (textures.albedoMap != nullptr)
    {
        mMaterialTextures.albedoMap = textures.albedoMap;
        mMaterialParams.textureFlag.useAlbedoMap = true;
    }
    if (textures.normalMap != nullptr)
    {
        mMaterialTextures.normalMap = textures.albedoMap;
        mMaterialParams.textureFlag.useNormalMap = true;
    }
    if (textures.metallicRoughnessMap != nullptr)
    {
        mMaterialTextures.metallicRoughnessMap = textures.albedoMap;
        mMaterialParams.textureFlag.useMetallicRoughnessMap = true;
    }
    if (textures.aoMap != nullptr)
    {
        mMaterialTextures.aoMap = textures.albedoMap;
        mMaterialParams.textureFlag.useAOMap = true;
    }
    if (textures.emissiveMap != nullptr)
    {
        mMaterialTextures.emissiveMap = textures.albedoMap;
        mMaterialParams.textureFlag.useEmissiveMap = true;
    }
}
void PBRMaterial::SetPBRTextures(std::optional<std::filesystem::path> albedoMapPath,
                                 std::optional<std::filesystem::path> normalMapPath,
                                 std::optional<std::filesystem::path> metallicRoughnessMapPath,
                                 std::optional<std::filesystem::path> aoMapPath,
                                 std::optional<std::filesystem::path> emissiveMapPath)
{
    if (albedoMapPath.has_value())
    {
        mMaterialTextures.albedoMap = mTextureManager->GetTexture(albedoMapPath.value());
        mMaterialParams.textureFlag.useAlbedoMap = true;
    }
    if (normalMapPath.has_value())
    {
        mMaterialTextures.normalMap = mTextureManager->GetTexture(normalMapPath.value());
        mMaterialParams.textureFlag.useNormalMap = true;
    }
    if (metallicRoughnessMapPath.has_value())
    {
        mMaterialTextures.metallicRoughnessMap = mTextureManager->GetTexture(metallicRoughnessMapPath.value());
        mMaterialParams.textureFlag.useMetallicRoughnessMap = true;
    }
    if (aoMapPath.has_value())
    {
        mMaterialTextures.aoMap = mTextureManager->GetTexture(aoMapPath.value());
        mMaterialParams.textureFlag.useAOMap = true;
    }
    if (emissiveMapPath.has_value())
    {
        mMaterialTextures.emissiveMap = mTextureManager->GetTexture(emissiveMapPath.value());
        mMaterialParams.textureFlag.useEmissiveMap = true;
    }
}
void PBRMaterial::Update()
{
    auto pbrDescriptorLayoutBindings = mPipelineLayoutManager->GetPBRDescriptorLayoutBindings();
    // 更新UBO
    auto mapped = mMaterialParamsUBO->GetAllocationInfo().pMappedData;
    memcpy(mapped, &mMaterialParams, sizeof(PBRMaterialParams));
    // 更新描述符集
    vk::WriteDescriptorSet writer;
    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.setBuffer(mMaterialParamsUBO->GetHandle()).setOffset(0).setRange(sizeof(PBRMaterialParams));
    writer.setDstSet(mMaterialDescriptorSet.get())
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setBufferInfo({bufferInfo})
        .setDstBinding(pbrDescriptorLayoutBindings.mParameterBinding.binding)
        .setDstArrayElement(0);
    mContext->GetDevice().updateDescriptorSets(writer, {});
    std::vector<vk::WriteDescriptorSet> writers;
    // 更新描述符集
    // Albedo Map
    vk::DescriptorImageInfo AlbedoImageInfo;
    AlbedoImageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
        .setImageView(mMaterialTextures.albedoMap->GetImageView())
        .setSampler(mMaterialTextures.albedoMap->GetSampler());
    vk::WriteDescriptorSet AlbedoWriter;
    AlbedoWriter.setDstSet(mMaterialDescriptorSet.get())
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setImageInfo({AlbedoImageInfo})
        .setDstBinding(pbrDescriptorLayoutBindings.mBaseColorBinding.binding)
        .setDstArrayElement(0);
    writers.push_back(AlbedoWriter);

    // Normal Map
    vk::DescriptorImageInfo NormalImageInfo;
    NormalImageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
        .setImageView(mMaterialTextures.normalMap->GetImageView())
        .setSampler(mMaterialTextures.normalMap->GetSampler());
    vk::WriteDescriptorSet NormalWriter;
    NormalWriter.setDstSet(mMaterialDescriptorSet.get())
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setImageInfo({NormalImageInfo})
        .setDstBinding(pbrDescriptorLayoutBindings.mNormalMapBinding.binding)
        .setDstArrayElement(0);
    writers.push_back(NormalWriter);

    // MetallicRoughness Map
    vk::DescriptorImageInfo MetallicRoughnessImageInfo;
    MetallicRoughnessImageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
        .setImageView(mMaterialTextures.metallicRoughnessMap->GetImageView())
        .setSampler(mMaterialTextures.metallicRoughnessMap->GetSampler());
    vk::WriteDescriptorSet MetallicRoughnessWriter;
    MetallicRoughnessWriter.setDstSet(mMaterialDescriptorSet.get())
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setImageInfo({MetallicRoughnessImageInfo})
        .setDstBinding(pbrDescriptorLayoutBindings.mMetallicRoughnessBinding.binding)
        .setDstArrayElement(0);
    writers.push_back(MetallicRoughnessWriter);
    // AO Map
    vk::DescriptorImageInfo AOImageInfo;
    AOImageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
        .setImageView(mMaterialTextures.aoMap->GetImageView())
        .setSampler(mMaterialTextures.aoMap->GetSampler());
    vk::WriteDescriptorSet AOWriter;
    AOWriter.setDstSet(mMaterialDescriptorSet.get())
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setImageInfo({AOImageInfo})
        .setDstBinding(pbrDescriptorLayoutBindings.mAmbientOcclusionBinding.binding)
        .setDstArrayElement(0);
    writers.push_back(AOWriter);
    // Emissive Map
    vk::DescriptorImageInfo EmissiveImageInfo;
    EmissiveImageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
        .setImageView(mMaterialTextures.emissiveMap->GetImageView())
        .setSampler(mMaterialTextures.emissiveMap->GetSampler());
    vk::WriteDescriptorSet EmissiveWriter;
    EmissiveWriter.setDstSet(mMaterialDescriptorSet.get())
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setImageInfo({EmissiveImageInfo})
        .setDstBinding(pbrDescriptorLayoutBindings.mEmissiveBinding.binding)
        .setDstArrayElement(0);
    writers.push_back(EmissiveWriter);

    mContext->GetDevice().updateDescriptorSets(writers, {});
}
} // namespace MEngine