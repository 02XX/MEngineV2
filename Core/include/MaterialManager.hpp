#pragma once
#include "Configure.hpp"
#include "Context.hpp"
#include "DescriptorManager.hpp"
#include "MEngine.hpp"
#include "Material/Interface/IMaterial.hpp"
#include "Material/PBRMaterial.hpp"
#include "NoCopyable.hpp"
#include "PipelineLayoutManager.hpp"
#include "PipelineManager.hpp"
#include "SamplerManager.hpp"
#include "Texture.hpp"
#include "TextureManager.hpp"
#include "magic_enum/magic_enum.hpp"
#include "nlohmann/json.hpp"
#include "stb_image.h"
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

namespace MEngine
{
class MaterialManager : public NoCopyable
{
  private:
    // DI
    std::shared_ptr<ILogger> mLogger;
    std::shared_ptr<Context> mContext;
    std::shared_ptr<PipelineManager> mPipelineManager;
    std::shared_ptr<PipelineLayoutManager> mPipelineLayoutManager;
    std::shared_ptr<DescriptorManager> mDescriptorManager;
    std::shared_ptr<SamplerManager> mSamplerManager;
    std::shared_ptr<TextureManager> mTextureManager;
    std::shared_ptr<BufferFactory> mBufferFactory;

  private:
    //{Id, Material}
    std::unordered_map<uint32_t, std::shared_ptr<IMaterial>> mMaterials; // 材质

  public:
    MaterialManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                    std::shared_ptr<PipelineManager> pipelineManager,
                    std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager,
                    std::shared_ptr<DescriptorManager> descriptorManager,
                    std::shared_ptr<SamplerManager> samplerManager, std::shared_ptr<TextureManager> textureManager,
                    std::shared_ptr<BufferFactory> bufferFactory);
    std::shared_ptr<IMaterial> GetMaterial(uint32_t id);
    uint32_t CreateMaterial(std::filesystem::path materialPath);
    void SaveMaterial(std::filesystem::path materialPath, std::shared_ptr<IMaterial> material);
};
} // namespace MEngine
