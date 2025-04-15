#pragma once
#include "Context.hpp"
#include "Entity/Interface/IEntity.hpp"
#include "Entity/Texture.hpp"
#include "Interface/IConfigure.hpp"
#include "Interface/ILogger.hpp"
#include "Repository/Repository.hpp"
#include "stb_image.h"
#include <memory>

namespace MEngine
{
class TextureRepository final : public Repository<Texture>
{
  private:
    // DI
    std::shared_ptr<ImageFactory> mImageFactory;
    std::shared_ptr<SamplerManager> mSamplerManager;

  private:
    std::shared_ptr<Texture> defaultTexture; // 默认纹理
  public:
    TextureRepository(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                      std::shared_ptr<IConfigure> configure, std::shared_ptr<ImageFactory> imageFactory,
                      std::shared_ptr<SamplerManager> samplerManager);
    Texture *Create() override;
    bool Update(const UUID &id, const Texture &delta) override;
    bool CheckValidate(const std::filesystem::path &filePath) const override;
    bool CheckValidate(const Texture &delta) const override;
};
} // namespace MEngine