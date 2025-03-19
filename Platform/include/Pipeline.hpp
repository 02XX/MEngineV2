#pragma once

#include "Context.hpp"
#include "MEngine.hpp"
#include "VMA.hpp"
#include <memory>
#include <vulkan/vulkan.hpp>
namespace MEngine
{
class MENGINE_API Pipeline final
{
  public:
    Pipeline(vk::Pipeline pipeline);
    Pipeline(const Pipeline &) = delete;
    Pipeline &operator=(const Pipeline &) = delete;
    Pipeline(Pipeline &&other) noexcept;
    Pipeline &operator=(Pipeline &&other) noexcept;
    ~Pipeline();

    const vk::Pipeline &GetPipeline() const;

  private:
    vk::Pipeline mPipeline;

    void Release();
};

using UniquePipeline = std::unique_ptr<Pipeline>;
using SharedPipeline = std::shared_ptr<Pipeline>;

} // namespace MEngine