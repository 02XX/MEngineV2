#pragma once

#include "Context.hpp"
#include "MEngine.hpp"
#include "VMA.hpp"
#include <memory>
#include <vulkan/vulkan.hpp>
namespace MEngine
{
class MENGINE_API PipelineLayout final
{
  public:
    PipelineLayout(vk::PipelineLayout pipelineLayout);
    PipelineLayout(const PipelineLayout &) = delete;
    PipelineLayout &operator=(const PipelineLayout &) = delete;
    PipelineLayout(PipelineLayout &&other) noexcept;
    PipelineLayout &operator=(PipelineLayout &&other) noexcept;
    ~PipelineLayout();

    const vk::Pipeline &GetPipeline() const;

  private:
    vk::PipelineLayout mPipelineLayout;

    void Release();
};
using UniquePipelineLayout = std::unique_ptr<PipelineLayout>;
using SharedPipelineLayout = std::shared_ptr<PipelineLayout>;

} // namespace MEngine