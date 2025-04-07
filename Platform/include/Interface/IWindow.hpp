#pragma once
#include "functional"
#include <vector>
#include <vulkan/vulkan.hpp>
namespace MEngine
{

class IWindow
{
  public:
    virtual ~IWindow() = default;
    // Vulkan
    virtual vk::SurfaceKHR GetSurface(vk::Instance instance) const = 0;
    virtual std::vector<const char *> GetInstanceRequiredExtensions() const = 0;

    virtual void PollEvents() = 0;
    virtual bool ShouldClose() const = 0;

    // 窗口属性
    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;

    using EventCallback = std::function<void(const void *event)>;
    virtual void SetEventCallback(EventCallback callback) = 0;
    // 扩展：获取平台相关句柄（如 SDL_Window*）
    virtual void *GetNativeHandle() const = 0;
};
} // namespace MEngine