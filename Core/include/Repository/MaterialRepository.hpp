#pragma once
#include "Context.hpp"
#include "Entity/Interface/IMaterial.hpp"

#include "Entity/Texture.hpp"

#include "DescriptorManager.hpp"
#include "PipelineLayoutManager.hpp"
#include "PipelineManager.hpp"
#include "Repository/TextureRepository.hpp"
#include "SamplerManager.hpp"

#include "Entity/Interface/IMaterial.hpp"
#include "Repository/Repository.hpp"
#include "stb_image.h"

#include "Entity/PBRMaterial.hpp"
namespace MEngine
{
class MaterialRepository : public Repository<PBRMaterial>
{
  private:
    // DI
    std::shared_ptr<PipelineManager> mPipelineManager;
    std::shared_ptr<PipelineLayoutManager> mPipelineLayoutManager;
    std::shared_ptr<DescriptorManager> mDescriptorManager;
    std::shared_ptr<SamplerManager> mSamplerManager;
    std::shared_ptr<TextureRepository> mTextureRepository;
    std::shared_ptr<BufferFactory> mBufferFactory;

  public:
    MaterialRepository(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                       std::shared_ptr<IConfigure> configure, std::shared_ptr<PipelineManager> pipelineManager,
                       std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager,
                       std::shared_ptr<DescriptorManager> descriptorManager,
                       std::shared_ptr<SamplerManager> samplerManager,
                       std::shared_ptr<TextureRepository> textureRepository,
                       std::shared_ptr<BufferFactory> bufferFactory);
    bool Update(const UUID &id, const PBRMaterial &delta) override;
    bool CheckValidate(const std::filesystem::path &filePath) const override;
    bool CheckValidate(const PBRMaterial &delta) const override;
};
} // namespace MEngine
