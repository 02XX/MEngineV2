#pragma once
#include "Entity/Interface/IMaterial.hpp"

namespace MEngine
{
struct MaterialComponent
{
    IMaterial *material; // 材质
};
} // namespace MEngine