#include "ResourceManager.hpp"
#include "PipelineManager.hpp"

namespace MEngine
{
void ResourceManager::LoadPipeline()
{
    // ForwardOpaque
    GraphicsPipelineConfig forwardOpaqueConfig;

    UniquePipeline forwardOpaque = mPipelineManager.CreateUniqueGraphicsPipeline(forwardOpaqueConfig);
    mPipelines[PipelineType::ForwardOpaque] = std::move(forwardOpaque);
    LogD("ForwardOpaque pipeline loaded.");
}
} // namespace MEngine