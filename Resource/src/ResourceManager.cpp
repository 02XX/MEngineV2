#include "ResourceManager.hpp"

namespace MEngine
{
ResourceManager::ResourceManager()
{
}

vk::Buffer ResourceManager::GetBuffer(uint32_t id)
{
    return mBuffers[id]->GetBuffer();
}

vk::Image ResourceManager::GetImage(uint32_t id)
{
    return mImages[id]->GetImage();
}

} // namespace MEngine