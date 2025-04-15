#include "Repository/PBRMaterialRepository.hpp"

namespace MEngine
{
PBRMaterialRepository::PBRMaterialRepository(
    std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context, std::shared_ptr<IConfigure> configure,
    std::shared_ptr<PipelineManager> pipelineManager, std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager,
    std::shared_ptr<DescriptorManager> descriptorManager, std::shared_ptr<SamplerManager> samplerManager,
    std::shared_ptr<TextureRepository> textureManager, std::shared_ptr<BufferFactory> bufferFactory)
    : Repository<PBRMaterial>(logger, context, configure), mPipelineManager(pipelineManager),
      mPipelineLayoutManager(pipelineLayoutManager), mDescriptorManager(descriptorManager),
      mSamplerManager(samplerManager), mTextureRepository(textureManager), mBufferFactory(bufferFactory)
{
}
bool PBRMaterialRepository::Update(const UUID &id, const PBRMaterial &delta)
{
    if (!CheckValidate(delta))
    {
        return false;
    }
    auto it = mEntities.find(id);
    auto material = Get(id);
    if (it != mEntities.end())
    {
        material->mMaterialParams = delta.mMaterialParams;
        material->mRenderType = delta.mRenderType;
        material->mAlbedoMapID = delta.mAlbedoMapID;
        material->mNormalMapID = delta.mNormalMapID;
        material->mMetallicRoughnessMapID = delta.mMetallicRoughnessMapID;
        material->mAOMapID = delta.mAOMapID;
        material->mEmissiveMapID = delta.mEmissiveMapID;
        // 更新材质的UBO
        auto pbrDescriptorLayoutBindings = mPipelineLayoutManager->GetPBRDescriptorLayoutBindings();
        auto mapped = material->mMaterialParamsUBO->GetAllocationInfo().pMappedData;
        memcpy(mapped, &material->mMaterialParams, sizeof(PBRParams));
        // 更新材质的DescriptorSet
        vk::WriteDescriptorSet writer;
        vk::DescriptorBufferInfo bufferInfo;
        bufferInfo.setBuffer(material->mMaterialParamsUBO->GetHandle()).setOffset(0).setRange(sizeof(PBRParams));
        writer.setDstSet(material->mMaterialDescriptorSet.get())
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
            .setImageView(mTextureRepository->Get(material->mAlbedoMapID)->GetImageView())
            .setSampler(mTextureRepository->Get(material->mAlbedoMapID)->GetSampler());
        vk::WriteDescriptorSet AlbedoWriter;
        AlbedoWriter.setDstSet(material->mMaterialDescriptorSet.get())
            .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
            .setImageInfo({AlbedoImageInfo})
            .setDstBinding(pbrDescriptorLayoutBindings.mBaseColorBinding.binding)
            .setDstArrayElement(0);
        writers.push_back(AlbedoWriter);

        // Normal Map
        vk::DescriptorImageInfo NormalImageInfo;
        NormalImageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setImageView(mTextureRepository->Get(material->mNormalMapID)->GetImageView())
            .setSampler(mTextureRepository->Get(material->mNormalMapID)->GetSampler());
        vk::WriteDescriptorSet NormalWriter;
        NormalWriter.setDstSet(material->mMaterialDescriptorSet.get())
            .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
            .setImageInfo({NormalImageInfo})
            .setDstBinding(pbrDescriptorLayoutBindings.mNormalMapBinding.binding)
            .setDstArrayElement(0);
        writers.push_back(NormalWriter);

        // MetallicRoughness Map
        vk::DescriptorImageInfo MetallicRoughnessImageInfo;
        MetallicRoughnessImageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setImageView(mTextureRepository->Get(material->mMetallicRoughnessMapID)->GetImageView())
            .setSampler(mTextureRepository->Get(material->mMetallicRoughnessMapID)->GetSampler());
        vk::WriteDescriptorSet MetallicRoughnessWriter;
        MetallicRoughnessWriter.setDstSet(material->mMaterialDescriptorSet.get())
            .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
            .setImageInfo({MetallicRoughnessImageInfo})
            .setDstBinding(pbrDescriptorLayoutBindings.mMetallicRoughnessBinding.binding)
            .setDstArrayElement(0);
        writers.push_back(MetallicRoughnessWriter);
        // AO Map
        vk::DescriptorImageInfo AOImageInfo;
        AOImageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setImageView(mTextureRepository->Get(material->mAOMapID)->GetImageView())
            .setSampler(mTextureRepository->Get(material->mAOMapID)->GetSampler());
        vk::WriteDescriptorSet AOWriter;
        AOWriter.setDstSet(material->mMaterialDescriptorSet.get())
            .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
            .setImageInfo({AOImageInfo})
            .setDstBinding(pbrDescriptorLayoutBindings.mAmbientOcclusionBinding.binding)
            .setDstArrayElement(0);
        writers.push_back(AOWriter);
        // Emissive Map
        vk::DescriptorImageInfo EmissiveImageInfo;
        EmissiveImageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setImageView(mTextureRepository->Get(material->mEmissiveMapID)->GetImageView())
            .setSampler(mTextureRepository->Get(material->mEmissiveMapID)->GetSampler());
        vk::WriteDescriptorSet EmissiveWriter;
        EmissiveWriter.setDstSet(material->mMaterialDescriptorSet.get())
            .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
            .setImageInfo({EmissiveImageInfo})
            .setDstBinding(pbrDescriptorLayoutBindings.mEmissiveBinding.binding)
            .setDstArrayElement(0);
        writers.push_back(EmissiveWriter);

        mContext->GetDevice().updateDescriptorSets(writers, {});
        return true;
    }
    mLogger->Info("Material with ID {} not exist", id);
    return false;
}
bool PBRMaterialRepository::CheckValidate(const std::filesystem::path &filePath) const
{
    if (filePath.empty())
    {
        mLogger->Error("Material path is empty!");
        return false;
    }
    if (std::filesystem::exists(filePath) == false)
    {
        mLogger->Error("Material path is not exist!");
        return false;
    }
    if (std::filesystem::is_directory(filePath))
    {
        mLogger->Error("Material path is a directory!");
        return false;
    }
    if (filePath.extension() != ".mat")
    {
        mLogger->Error("Material path is not a .mat file!");
        return false;
    }
    return true;
}
bool PBRMaterialRepository::CheckValidate(const PBRMaterial &delta) const
{
    return true;
}
// std::shared_ptr<IMaterial> PBRMaterialRepository::GetMaterial(std::filesystem::path materialPath)
// {
//     // 判断path的extension是否为.mat
//     if (materialPath.extension() != ".mat")
//     {
//         // 修改为.mat
//         materialPath.replace_extension(".mat");
//     }
//     auto id = std::hash<std::string>{}(materialPath.string());
//     auto result = GetMaterial(id);
//     if (result != nullptr)
//     {
//         return result;
//     }
//     // 如果没有找到，则尝试从文件中加载材质
//     id = CreateMaterial(materialPath);
//     result = GetMaterial(id);
//     return result;
// }
// std::shared_ptr<IMaterial> PBRMaterialRepository::GetMaterial(uint32_t id)
// {
//     auto it = mMaterials.find(id);
//     if (it != mMaterials.end())
//     {
//         return it->second;
//     }
//     return nullptr;
// }
// void PBRMaterialRepository::SaveMaterial(std::filesystem::path materialPath, std::shared_ptr<IMaterial> material)
// {
//     // 判断path的extension是否为.mat
//     if (materialPath.extension() != ".mat")
//     {
//         // 修改为.mat
//         materialPath.replace_extension(".mat");
//     }
//     Json materialJson;
//     switch (material->GetPipelineType())
//     {
//     case PipelineType::ShadowMap:
//         break;
//     case PipelineType::ForwardOpaque:
//     case PipelineType::ForwardTransparent:
//     case PipelineType::DeferredGBuffer:
//     case PipelineType::DeferredLighting:
//         materialJson = std::static_pointer_cast<PBRMaterial>(material)->GetPBRMaterialParams();
//         break;
//     case PipelineType::ScreenSpaceEffectSSAO:
//     case PipelineType::ScreenSpaceEffectSSR:
//     case PipelineType::SkinnedMesh:
//     case PipelineType::MorphTarget:
//     case PipelineType::ParticleCPU:
//     case PipelineType::ParticleGPU:
//     case PipelineType::Decal:
//     case PipelineType::PostProcessToneMapping:
//     case PipelineType::PostProcessBloom:
//     case PipelineType::PostProcessDOF:
//     case PipelineType::PostProcessMotionBlur:
//     case PipelineType::PostProcessFXAA:
//     case PipelineType::PostProcessSMAA:
//     case PipelineType::PostProcessVignette:
//     case PipelineType::PostProcessChromaticAberration:
//     case PipelineType::PostProcessFilmGrain:
//     case PipelineType::PostProcessColorGrading:
//     case PipelineType::UISprite:
//     case PipelineType::UIText:
//     case PipelineType::Toon:
//     case PipelineType::Wireframe:
//         break;
//     default:
//         mLogger->Error("Unsupported material type for saving: {}",
//         magic_enum::enum_name(material->GetPipelineType())); return;
//     }
//     std::ofstream file(materialPath);
//     if (!file.is_open())
//     {
//         mLogger->Error("Failed to open material file for writing: {}", materialPath.string());
//         return;
//     }
//     file << std::setw(4) << materialJson << std::endl;
//     file.close();
//     mLogger->Info("Material saved successfully: {}", materialPath.string());
// }
// uint32_t PBRMaterialRepository::CreateMaterial(std::filesystem::path materialPath)
// {
//     // 判断path的extension是否为.mat
//     if (materialPath.extension() != ".mat")
//     {
//         // 修改为.mat
//         materialPath.replace_extension(".mat");
//     }
//     auto id = std::hash<std::string>{}(materialPath.string());

//     std::shared_ptr<IMaterial> material =
//         std::make_shared<PBRMaterial>(mContext, mTextureRepository, mBufferFactory, mPipelineLayoutManager,
//                                       mDescriptorManager, materialPath.stem().string(), id, materialPath);
//     mMaterials[id] = material;
//     SaveMaterial(materialPath, material);
//     return id;
// }
// std::vector<std::shared_ptr<IMaterial>> PBRMaterialRepository::GetAllMaterials() const
// {
//     std::vector<std::shared_ptr<IMaterial>> materials;
//     for (auto &material : mMaterials)
//     {
//         materials.push_back(material.second);
//     }
//     return materials;
// }
// uint32_t PBRMaterialRepository::LoadMaterialFromFile(std::filesystem::path materialPath)
// {
//     auto id = std::hash<std::string>{}(materialPath.string());
//     if (mMaterials.find(id) != mMaterials.end())
//     {
//         mLogger->Info("Material already exists: {}", materialPath.string());
//         return id;
//     }
//     std::ifstream file(materialPath);
//     if (!file.is_open())
//     {
//         mLogger->Error("Failed to open material file: {}", materialPath.string());
//         return 0;
//     }
//     Json materialJson = Json::parse(file);
//     file.close();
//     auto pipelineTypeOpt = magic_enum::enum_cast<PipelineType>(materialJson["pipelineType"].get<std::string>());
//     if (!pipelineTypeOpt.has_value())
//     {
//         mLogger->Error("Invalid pipeline type in material file: {}", materialPath.string());
//         return 0;
//     }
//     std::shared_ptr<IMaterial> material;
//     switch (pipelineTypeOpt.value())
//     {
//     case PipelineType::ShadowMap:
//         break;
//     case PipelineType::ForwardOpaque:
//     case PipelineType::ForwardTransparent:
//     case PipelineType::DeferredGBuffer:
//     case PipelineType::DeferredLighting: {
//         PBRMaterialParams pbrMaterialParams = materialJson.get<PBRMaterialParams>();
//         material = std::make_shared<PBRMaterial>(mContext, mTextureRepository, mBufferFactory,
//         mPipelineLayoutManager,
//                                                  mDescriptorManager, pbrMaterialParams);
//         material->Update();
//         break;
//     }
//     case PipelineType::ScreenSpaceEffectSSAO:
//     case PipelineType::ScreenSpaceEffectSSR:
//     case PipelineType::SkinnedMesh:
//     case PipelineType::MorphTarget:
//     case PipelineType::ParticleCPU:
//     case PipelineType::ParticleGPU:
//     case PipelineType::Decal:
//     case PipelineType::PostProcessToneMapping:
//     case PipelineType::PostProcessBloom:
//     case PipelineType::PostProcessDOF:
//     case PipelineType::PostProcessMotionBlur:
//     case PipelineType::PostProcessFXAA:
//     case PipelineType::PostProcessSMAA:
//     case PipelineType::PostProcessVignette:
//     case PipelineType::PostProcessChromaticAberration:
//     case PipelineType::PostProcessFilmGrain:
//     case PipelineType::PostProcessColorGrading:
//     case PipelineType::UISprite:
//     case PipelineType::UIText:
//     case PipelineType::Toon:
//     case PipelineType::Wireframe:
//         break;
//     default:
//         mLogger->Error("Unsupported material type for loading: {}", magic_enum::enum_name(pipelineTypeOpt.value()));
//         return 0;
//     }
//     material->SetMaterialID(id);
//     material->SetMaterialName(materialPath.stem().string());
//     mMaterials[id] = material;
//     mLogger->Info("Material loaded successfully: {}", materialPath.string());
//     return id;
// }

} // namespace MEngine