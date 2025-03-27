#pragma once
#include "MEngine.hpp"
#include "Material.hpp"
namespace MEngine
{
struct MENGINE_API MaterialComponent
{
    std::shared_ptr<Material> material;
};
} // namespace MEngine