#pragma once
#include "Buffer.hpp"
#include "Context.hpp"
#include "Logger.hpp"
#include "MEngine.hpp"
#include "SharedHandle.hpp"
#include <cstdint>
namespace MEngine
{
class MENGINE_API MaterialComponent final
{
  private:
    SharedPipelineLayout mPipelineLayout;
    SharedPipeline mPipeline;
    SharedDescriptorSet mDescriptorSet;
    uint32_t mMaterialID;

  public:
    MaterialComponent();

    void SetPipeline(SharedPipeline pipeline);
    void SetPipelineLayout(SharedPipelineLayout layout);
    void SetDescriptorSet(SharedDescriptorSet set);
    uint32_t GetID() const noexcept;

    SharedPipeline GetPipeline() const noexcept;
    SharedPipelineLayout GetPipelineLayout() const noexcept;
    SharedDescriptorSet GetDescriptorSet() const noexcept;
};
} // namespace MEngine