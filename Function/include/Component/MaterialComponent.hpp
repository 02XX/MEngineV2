#pragma once
#include "Component/Interface/IComponent.hpp"
#include "Entity/Interface/IMaterial.hpp"
#include "entt/entity/storage.hpp"
namespace MEngine
{
struct MaterialComponent : public IComponent<>
{
    IMaterial *material = nullptr; // 材质
    MaterialComponent(IMaterial *material) : material(material)
    {
    }
    MaterialComponent() = default;
};
} // namespace MEngine

// template <> struct entt::storage_type<MEngine::MaterialComponent>
// {
//     using type = entt::storage<MEngine::MaterialComponent>;
// };