#include "Pipeline.hpp"

namespace MEngine
{
Pipeline::Pipeline(vk::Pipeline pipeline) : mPipeline(pipeline)
{
}
Pipeline::Pipeline(Pipeline &&other) noexcept : mPipeline(other.mPipeline)
{
    other.mPipeline = nullptr;
}
Pipeline &Pipeline::operator=(Pipeline &&other) noexcept
{
    if (this != &other)
    {
        Release();
        mPipeline = other.mPipeline;
        other.mPipeline = nullptr;
    }
    return *this;
}
Pipeline::~Pipeline()
{
    Release();
}

const vk::Pipeline &Pipeline::GetPipeline() const
{
    return mPipeline;
}

void Pipeline::Release()
{
    auto &context = Context::Instance();
    context.GetDevice()->destroyPipeline(mPipeline);
    LogT("Pipeline Destroyed.");
}
} // namespace MEngine