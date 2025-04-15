#pragma once
#include "Entity/Interface/IMaterial.hpp"
#include "MEngine.hpp"
#include "PipelineLayoutManager.hpp"
#include "PipelineManager.hpp"
#include <memory>

namespace MEngine
{
struct MaterialComponent
{
    std::shared_ptr<IMaterial> material; // 材质
};
} // namespace MEngine