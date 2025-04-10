#pragma once
#include "DescriptorManager.hpp"
#include "NoCopyable.hpp"
#include "PipelineLayoutManager.hpp"
#include "PipelineManager.hpp"
#include "Texture.hpp"
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>
namespace MEngine
{
class Material final : public NoCopyable
{
  private:
    // DI
    std::shared_ptr<DescriptorManager> mDescriptorManager;         // 描述符管理器
    std::shared_ptr<PipelineManager> mPipelineManager;             // 管线管理器
    std::shared_ptr<PipelineLayoutManager> mPipelineLayoutManager; // 管线布局管理器
  private:
    PipelineType mPipelineType;                                          // 渲染管线类型
    PipelineLayoutType mPipelineLayoutType;                              // 管线布局类型
    vk::UniqueDescriptorSet mDescriptorSet;                              // 描述符集
    std::unordered_map<TextureType, std::shared_ptr<Texture>> mTextures; // 纹理

  public:
    Material(std::shared_ptr<DescriptorManager> descriptorManager, std::shared_ptr<PipelineManager> pipelineManager,
             std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager, PipelineType pipelineType,
             PipelineLayoutType pipelineLayoutType);

    void AddTexture(TextureType type, std::shared_ptr<Texture> texture);
    inline PipelineType GetPipelineType() const
    {
        return mPipelineType;
    }
    inline PipelineLayoutType GetPipelineLayoutType() const
    {
        return mPipelineLayoutType;
    }
    inline vk::DescriptorSet GetDescriptorSet() const
    {
        return mDescriptorSet.get();
    }
};
} // namespace MEngine