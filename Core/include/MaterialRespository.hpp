#pragma once
#include "Configure.hpp"
#include "Context.hpp"
#include "Entity/Interface/IMaterial.hpp"
#include "Entity/PBRMaterial.hpp"
#include "Entity/Texture.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"

#include "DescriptorManager.hpp"
#include "PipelineLayoutManager.hpp"
#include "PipelineManager.hpp"
#include "Repository/Interface/IRepository.hpp"
#include "SamplerManager.hpp"
#include "TextureManager.hpp"

#include "magic_enum/magic_enum.hpp"
#include "nlohmann/json.hpp"
#include "stb_image.h"

namespace MEngine
{
class MaterialRepository : public IRepository<PBRMaterial>
{
  private:
    // DI
    // std::shared_ptr<ILogger> mLogger;
    // std::shared_ptr<Context> mContext;
    // std::shared_ptr<PipelineManager> mPipelineManager;
    // std::shared_ptr<PipelineLayoutManager> mPipelineLayoutManager;
    // std::shared_ptr<DescriptorManager> mDescriptorManager;
    // std::shared_ptr<SamplerManager> mSamplerManager;
    // std::shared_ptr<TextureManager> mTextureManager;
    // std::shared_ptr<BufferFactory> mBufferFactory;

  private:
    // TODO: 接入数据库管理，接入Redis管理
    //{Id, Material}
    std::unordered_map<uint32_t, std::shared_ptr<IMaterial>> mMaterials; // 材质库

  public:
    // MaterialRepository(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
    //                    std::shared_ptr<PipelineManager> pipelineManager,
    //                    std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager,
    //                    std::shared_ptr<DescriptorManager> descriptorManager,
    //                    std::shared_ptr<SamplerManager> samplerManager, std::shared_ptr<TextureManager>
    //                    textureManager, std::shared_ptr<BufferFactory> bufferFactory);
};
} // namespace MEngine
