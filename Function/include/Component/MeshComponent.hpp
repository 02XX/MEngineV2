#pragma once
#include "Buffer.hpp"
#include "Component/Interface/IComponent.hpp"
#include "MEngine.hpp"
#include "Mesh.hpp"
#include "entt/entity/storage.hpp"
namespace MEngine
{
struct MeshComponent : public IComponent<>
{
    std::shared_ptr<Mesh> mesh{};
    MeshComponent() = default;
    MeshComponent(std::shared_ptr<Mesh> mesh) : mesh(mesh)
    {
    }
};
} // namespace MEngine

// template <> struct entt::storage_type<MEngine::MeshComponent>
// {
//     using type = entt::storage<MEngine::MeshComponent>;
// };