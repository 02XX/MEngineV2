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
        return it->second;
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
uint32_t MaterialManager::LoadMaterial(std::filesystem::path materialPath)
{
    Json materialJson;
    std::ifstream file(materialPath);
    if (!file.is_open())
    {
        mLogger->Error("Failed to open material file: {}", materialPath.string());
        return 0;
    }
    try
    {
        materialJson = Json::parse(file);
        MaterialParams materialParams = materialJson.get<MaterialParams>();
        auto material = std::make_shared<Material>(mDescriptorManager, mPipelineManager, mPipelineLayoutManager,
                                                   materialParams.pipelineType, materialParams.pipelineLayoutType);
        if (materialParams.baseColorPath.has_value())
        {
            auto texture = mTextureManager->CreateTexture(materialParams.baseColorPath.value(), TextureType::BaseColor);
            material->AddTexture(texture);
        }
        if (materialParams.normalPath.has_value())
        {
            auto texture = mTextureManager->CreateTexture(materialParams.normalPath.value(), TextureType::Normal);
            material->AddTexture(texture);
        }
        if (materialParams.metallicPath.has_value())
        {
            auto texture = mTextureManager->CreateTexture(materialParams.metallicPath.value(), TextureType::Metallic);
            material->AddTexture(texture);
        }
        if (materialParams.roughnessPath.has_value())
        {
            auto texture = mTextureManager->CreateTexture(materialParams.roughnessPath.value(), TextureType::Roughness);
            material->AddTexture(texture);
        }
        if (materialParams.ambientOcclusionPath.has_value())
        {
            auto texture = mTextureManager->CreateTexture(materialParams.ambientOcclusionPath.value(),
                                                          TextureType::AmbientOcclusion);
            material->AddTexture(texture);
        }
        auto id = std::hash<std::string>{}(materialPath.string());
        mMaterials[id] = material;
        mLogger->Info("Material loaded successfully: {}", materialPath.string());
        return id;
    }
    catch (const Json::exception &e)
    {
        mLogger->Error("Failed to parse material file: {}. Error: {}", materialPath.string(), e.what());
        return 0;
    }
    return 0;
}
void MaterialManager::SaveMaterial(std::filesystem::path materialPath, std::shared_ptr<Material> material)
{
    MaterialParams materialParams;
    materialParams.pipelineType = material->GetPipelineType();
    materialParams.pipelineLayoutType = material->GetPipelineLayoutType();
    for (const auto &texture : material->GetTextures())
    {
        materialParams.texturePaths[texture->GetTextureType()] = texture->GetPath();
    }
    Json materialJson = materialParams;
    std::ofstream file(materialPath);
    if (!file.is_open())
    {
        mLogger->Error("Failed to open material file for writing: {}", materialPath.string());
        return;
    }
    file << std::setw(4) << materialJson << std::endl;
    mLogger->Info("Material saved successfully: {}", materialPath.string());
}
void MaterialManager::CreateMaterial(std::filesystem::path materialPath)
{
    MaterialParams materialParams{};
    Json materialJson = materialParams;
    std::ofstream file(materialPath);
    if (!file.is_open())
    {
        mLogger->Error("Failed to open material file for writing: {}", materialPath.string());
        return;
    }
    file << std::setw(4) << materialJson << std::endl;
    mLogger->Info("Material saved successfully: {}", materialPath.string());
}
} // namespace MEngine