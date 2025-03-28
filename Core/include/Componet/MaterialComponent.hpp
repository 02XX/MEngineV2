#pragma once
#include "MEngine.hpp"
#include "Material.hpp"
namespace MEngine
{
struct MaterialComponent
{
    std::shared_ptr<Material> material;
};
} // namespace MEngine