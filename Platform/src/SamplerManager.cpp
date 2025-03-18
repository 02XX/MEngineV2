#include "SamplerManager.hpp"

namespace MEngine
{
vk::UniqueSampler SamplerManagerManager::CreateSampler(vk::Filter magFilter, vk::Filter minFilter,
                                                       vk::SamplerMipmapMode mipmapMode,
                                                       vk::SamplerAddressMode addressMode, float mipLodBias,
                                                       vk::Bool32 anisotropyEnable, float maxAnisotropy,
                                                       vk::Bool32 compareEnable, vk::CompareOp compareOp, float minLod,
                                                       float maxLod, vk::BorderColor borderColor,
                                                       vk::Bool32 unnormalizedCoordinates)
{
    auto &context = Context::Instance();
    vk::SamplerCreateInfo samplerCreateInfo;
    samplerCreateInfo
        .setMagFilter(magFilter) // ​放大滤波：当纹理被放大时（纹素
                                 // <片段）的采样方式。可选值：eNearest（最近邻）、eLinear（线性插值）
        .setMinFilter(minFilter) // 缩小滤波：当纹理被缩小时（纹素 > 片段）的采样方式。可选值同上
        .setMipmapMode(mipmapMode) // Mipmap 滤波模式：eNearest（使用最近的mip层）、eLinear（在mip层间线性插值）
        .setAddressModeU(
            addressMode) // U 轴（水平）的寻址模式处理纹理坐标超出 [0,1]
                         // 范围时的行为：eRepeat（重复）、eMirroredRepeat（镜像重复）、eClampToEdge（钳制到边缘）、eClampToBorder（钳制到边界色）、eMirrorClampToEdge（镜像钳制到边缘）
        .setAddressModeV(addressMode) // V 轴（垂直）的寻址模式
        .setAddressModeW(addressMode) // W 轴（深度）的寻址模式
        .setMipLodBias(mipLodBias)    // mip层级偏移
        .setMinLod(minLod)            // 允许使用的最小 mipmap 层级（通常为0，即最大分辨率层）
        .setMaxLod(maxLod) // 允许使用的最大 mipmap 层级（设为 VK_LOD_CLAMP_NONE 表示无限制）
        .setAnisotropyEnable(anisotropyEnable) // 启用各向异性过滤,需检查 deviceFeatures.samplerAnisotropy 支持
        .setMaxAnisotropy(maxAnisotropy)       // 各向异性过滤的最大采样率有效范围 [1,
                                         // physicalDeviceProperties.limits.maxSamplerAnisotropy]常用值：4.0、8.0、16.0
        .setCompareEnable(
            compareEnable) // 启用深度比较：设为 VK_TRUE 时，将执行深度比较而非直接采样，用于阴影贴图等场景
        .setCompareOp(compareOp) // 比较运算符：定义深度比较方式，如 eLess、eGreater 等，与深度测试类似
        .setBorderColor(
            borderColor) // ​边界颜色：当 addressMode 设为 eClampToBorder
                         // 时的颜色：eFloatTransparentBlack、eIntOpaqueWhite 等需确保颜色格式与采样器使用场景兼容
        .setUnnormalizedCoordinates(
            unnormalizedCoordinates); // 是否使用非归一化坐标：设为 VK_TRUE 时，纹理坐标范围为 [0, texWidth]、[0,
                                      // texHeight]、[0, texDepth]，而非 [0, 1]
    auto sampler = context.GetDevice()->createSamplerUnique(samplerCreateInfo);
    LogD("Sampler created");
    return sampler;
}
} // namespace MEngine