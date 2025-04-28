#include "Repository/Texture2DRepository.hpp"

namespace MEngine
{
Texture2DRepository::Texture2DRepository(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                                         std::shared_ptr<IConfigure> configure,
                                         std::shared_ptr<ImageFactory> imageFactory,
                                         std::shared_ptr<SamplerManager> samplerManager)
    : Repository<Texture2D>(logger, context, configure), mImageFactory(imageFactory), mSamplerManager(samplerManager)
{
    mCheckBoardData = CheckBoard();
    auto defaultTexture = Create();
    auto id = defaultTexture->GetID();
    std::swap(mEntities[id], mEntities[UUID{}]);
    mEntities.erase(id); // 交换后记得删除原ID，因为[]会默认创建一个键值对{UUID, nullptr}
}
Texture2D *Texture2DRepository::Create()
{
    auto texture = std::make_unique<Texture2D>();
    texture->mWidth = 4096;
    texture->mHeight = 4096;
    texture->mChannels = 4;
    texture->mImage =
        mImageFactory->CreateImage(ImageType::Texture2D, vk::Extent3D{texture->mWidth, texture->mHeight, 1},
                                   mCheckBoardData.size(), mCheckBoardData.data());
    texture->mImageView = mImageFactory->CreateImageView(texture->mImage.get());
    texture->mSampler = mSamplerManager->CreateUniqueSampler(vk::Filter::eLinear, vk::Filter::eLinear);
    auto id = texture->GetID();
    mEntities[id] = std::move(texture);
    return mEntities[id].get();
}
bool Texture2DRepository::Update(const UUID &id, const Texture2D &delta)
{
    if (!CheckValidate(delta))
    {
        return false;
    }
    auto it = mEntities.find(id);
    auto texture = Get(id);
    if (it != mEntities.end())
    {
        it->second->imagePath = delta.imagePath;
        int width, height, channels;
        stbi_set_flip_vertically_on_load(true);
        auto imageData = stbi_load(delta.imagePath.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
        if (!imageData)
        {
            mLogger->Error("Failed to load default texture image: {}", delta.imagePath.string().c_str());
            return false;
        }
        texture->mWidth = static_cast<uint32_t>(width);
        texture->mHeight = static_cast<uint32_t>(height);
        texture->mChannels = 4;
        auto size = static_cast<vk::DeviceSize>(texture->mWidth * texture->mHeight * texture->mChannels);
        texture->mImage = mImageFactory->CreateImage(
            ImageType::Texture2D, vk::Extent3D{texture->mWidth, texture->mHeight, 1}, size, imageData);
        texture->mImageView = mImageFactory->CreateImageView(texture->mImage.get());
        texture->mSampler = mSamplerManager->CreateUniqueSampler(vk::Filter::eLinear, vk::Filter::eLinear);
        stbi_image_free(imageData);
        return true;
    }
    mLogger->Info("Texture with ID {} not exist", id);
    return false;
}
bool Texture2DRepository::CheckValidate(const std::filesystem::path &filePath) const
{
    if (filePath.empty())
    {
        mLogger->Error("Default texture path is empty!");
        return false;
    }
    if (std::filesystem::exists(filePath) == false)
    {
        mLogger->Error("Default texture path is not exist!");
        return false;
    }
    if (std::filesystem::is_directory(filePath))
    {
        mLogger->Error("Default texture path is a directory!");
        return false;
    }
    if (filePath.extension() != ".png")
    {
        mLogger->Error("Default texture path is not a png file!");
        return false;
    }
    return true;
}
bool Texture2DRepository::CheckValidate(const Texture2D &delta) const
{
    // Check imagePath
    return CheckValidate(delta.imagePath);
    // TODO: CheckValidate other members
}
std::vector<unsigned char> Texture2DRepository::CheckBoard()
{
    // 4k
    int width = 4096;
    int height = 4096;
    int channels = 4; // RGBA
    int grid = 8;
    int tileSize = width / grid;
    std::vector<unsigned char> checkBoard(width * height * channels, 0);
    for (int y = 0; y < height; y += tileSize)
    {
        for (int x = 0; x < width; x += tileSize)
        {
            unsigned char color = ((x / tileSize) % 2 == (y / tileSize) % 2) ? 255 : 0;
            for (int j = 0; j < tileSize && (y + j) < height; ++j)
            {
                for (int i = 0; i < tileSize && (x + i) < width; ++i)
                {
                    int index = ((y + j) * width + (x + i)) * channels;
                    checkBoard[index] = color;
                    checkBoard[index + 1] = color;
                    checkBoard[index + 2] = color;
                    checkBoard[index + 3] = 255; // Alpha channel
                }
            }
        }
    }
    return checkBoard;
}
} // namespace MEngine