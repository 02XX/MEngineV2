#pragma once
#include "Component/Interface/IComponent.hpp"
#include "Entity/Interface/IMaterial.hpp"
#include "entt/entity/storage.hpp"
namespace MEngine
{
struct MaterialComponent : public IComponent<>
{
    IMaterial *material; // 材质
};
} // namespace MEngine

// template <> struct entt::storage_type<MEngine::MaterialComponent>
// {
//     using type = entt::storage<MEngine::MaterialComponent>;
// };