#include "MaterialManager.hpp"

namespace MEngine
{
std::shared_ptr<Material> MaterialManager::CreateDefaultMaterial()
{
    auto defaultMaterial = std::make_shared<Material>(PipelineType::ForwardOpaque, nullptr);
    return defaultMaterial;
}
std::shared_ptr<Material> MaterialManager::GetMaterial(MaterialType type)
{
    auto it = mMaterials.find(type);
    if (it != mMaterials.end())
    {
        auto material = it->second.lock();
        if (material)
        {
            return material;
        }
    }

    if (type == MaterialType::Default)
    {
        auto defaultMaterial = CreateDefaultMaterial();
        mMaterials[type] = defaultMaterial;
        return defaultMaterial;
    }

    return nullptr;
}
} // namespace MEngine