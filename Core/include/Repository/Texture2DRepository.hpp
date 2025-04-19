#pragma once
#include "Context.hpp"
#include "Entity/Interface/IEntity.hpp"
#include "Entity/Texture2D.hpp"
#include "Interface/IConfigure.hpp"
#include "Interface/ILogger.hpp"
#include "Repository/Repository.hpp"
#include "stb_image.h"
#include <memory>
#include <vector>

namespace MEngine
{
class Texture2DRepository final : public Repository<Texture2D>
{
  private:
    // DI
    std::shared_ptr<ImageFactory> mImageFactory;
    std::shared_ptr<SamplerManager> mSamplerManager;

  private:
    std::vector<unsigned char> mCheckBoardData;

  public:
    Texture2DRepository(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                        std::shared_ptr<IConfigure> configure, std::shared_ptr<ImageFactory> imageFactory,
                        std::shared_ptr<SamplerManager> samplerManager);
    Texture2D *Create() override;
    bool Update(const UUID &id, const Texture2D &delta) override;
    bool CheckValidate(const std::filesystem::path &filePath) const override;
    bool CheckValidate(const Texture2D &delta) const override;
    std::vector<unsigned char> CheckBoard();
};
} // namespace MEngine