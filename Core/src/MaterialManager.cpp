#include "MaterialManager.hpp"

namespace MEngine
{
MaterialManager::MaterialManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                                 std::shared_ptr<PipelineManager> pipelineManager,
                                 std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager,
                                 std::shared_ptr<DescriptorManager> descriptorManager,
                                 std::shared_ptr<SamplerManager> samplerManager,
                                 std::shared_ptr<TextureManager> textureManager,
                                 std::shared_ptr<BufferFactory> bufferFactory)
    : mLogger(logger), mContext(context), mPipelineManager(pipelineManager),
      mPipelineLayoutManager(pipelineLayoutManager), mDescriptorManager(descriptorManager),
      mSamplerManager(samplerManager), mTextureManager(textureManager), mBufferFactory(bufferFactory)
{
}
std::shared_ptr<IMaterial> MaterialManager::GetMaterial(std::filesystem::path materialPath)
{
    // 判断path的extension是否为.mat
    if (materialPath.extension() != ".mat")
    {
        // 修改为.mat
        materialPath.replace_extension(".mat");
    }
    auto id = std::hash<std::string>{}(materialPath.string());
    auto result = GetMaterial(id);
    if (result != nullptr)
    {
        return result;
    }
    // 如果没有找到，则尝试从文件中加载材质
    id = CreateMaterial(materialPath);
    result = GetMaterial(id);
    return result;
}
std::shared_ptr<IMaterial> MaterialManager::GetMaterial(uint32_t id)
{
    auto it = mMaterials.find(id);
    if (it != mMaterials.end())
    {
        return it->second;
    }
    return nullptr;
}
void MaterialManager::SaveMaterial(std::filesystem::path materialPath, std::shared_ptr<IMaterial> material)
{
    // 判断path的extension是否为.mat
    if (materialPath.extension() != ".mat")
    {
        // 修改为.mat
        materialPath.replace_extension(".mat");
    }
    Json materialJson;
    switch (material->GetPipelineType())
    {
    case PipelineType::ShadowMap:
        break;
    case PipelineType::ForwardOpaque:
    case PipelineType::ForwardTransparent:
    case PipelineType::DeferredGBuffer:
    case PipelineType::DeferredLighting:
        materialJson = *std::static_pointer_cast<PBRMaterial>(material);
        break;
    case PipelineType::ScreenSpaceEffectSSAO:
    case PipelineType::ScreenSpaceEffectSSR:
    case PipelineType::SkinnedMesh:
    case PipelineType::MorphTarget:
    case PipelineType::ParticleCPU:
    case PipelineType::ParticleGPU:
    case PipelineType::Decal:
    case PipelineType::PostProcessToneMapping:
    case PipelineType::PostProcessBloom:
    case PipelineType::PostProcessDOF:
    case PipelineType::PostProcessMotionBlur:
    case PipelineType::PostProcessFXAA:
    case PipelineType::PostProcessSMAA:
    case PipelineType::PostProcessVignette:
    case PipelineType::PostProcessChromaticAberration:
    case PipelineType::PostProcessFilmGrain:
    case PipelineType::PostProcessColorGrading:
    case PipelineType::UISprite:
    case PipelineType::UIText:
    case PipelineType::Toon:
    case PipelineType::Wireframe:
        break;
    default:
        mLogger->Error("Unsupported material type for saving: {}", magic_enum::enum_name(material->GetPipelineType()));
        return;
    }
    std::ofstream file(materialPath);
    if (!file.is_open())
    {
        mLogger->Error("Failed to open material file for writing: {}", materialPath.string());
        return;
    }
    file << std::setw(4) << materialJson << std::endl;
    mLogger->Info("Material saved successfully: {}", materialPath.string());
}
uint32_t MaterialManager::CreateMaterial(std::filesystem::path materialPath)
{
    auto id = std::hash<std::string>{}(materialPath.string());

    std::shared_ptr<IMaterial> material = std::make_shared<PBRMaterial>(
        mContext, mTextureManager, mBufferFactory, mPipelineLayoutManager, mDescriptorManager, "DefaultMaterial", id);
    mMaterials[id] = material;
    SaveMaterial(materialPath, material);
    return id;
}
std::vector<std::shared_ptr<IMaterial>> MaterialManager::GetAllMaterials() const
{
    std::vector<std::shared_ptr<IMaterial>> materials;
    for (auto &material : mMaterials)
    {
        materials.push_back(material.second);
    }
    return materials;
}
} // namespace MEngine