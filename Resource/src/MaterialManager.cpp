// #include "MaterialManager.hpp"
// #include <vulkan/vulkan_handles.hpp>

// namespace MEngine
// {
// MaterialManager::MaterialManager()
// {
//     mDescriptorManager = std::make_unique<DescriptorManager>();
//     mImageManager = std::make_unique<ImageManager>();
//     // mSamplerManager = std::make_unique<SamplerManagerManager>();
// }
// std::shared_ptr<Material> MaterialManager::CreateDefaultMaterial()
// {
//     auto defaultMaterial = std::make_shared<Material>(PipelineType::DefferGBuffer, nullptr);
//     return defaultMaterial;
// }
// std::shared_ptr<Material> MaterialManager::CreatePBRMaterial()
// {
//     return nullptr;
//     // auto &context = Context::Instance();
//     // vk::DescriptorSetLayoutBinding binding;
//     // binding
//     //     .setBinding(1) // albedo
//     //     .setDescriptorCount(1)
//     //     .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
//     //     .setStageFlags(vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex)
//     //     .setPImmutableSamplers(nullptr);
//     // vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
//     // descriptorSetLayoutCreateInfo.setBindings({binding});
//     // auto descriptorSetLayout = context.GetDevice().createDescriptorSetLayoutUnique(descriptorSetLayoutCreateInfo);
//     // auto descriptorSet = mDescriptorManager->AllocateUniqueDescriptorSet({*descriptorSetLayout});
//     // vk::DescriptorSet deset = descriptorSet[0].release();
//     // auto pbrMaterial = std::make_shared<Material>(PipelineType::ForwardOpaque, deset);

//     // // 图像
//     // int width, height, channels;
//     // auto data = stbi_load("assets/bg.jpg", &width, &height, &channels, 0);
//     // if (!data)
//     // {
//     //     LogE("Failed to load texture: {}", stbi_failure_reason());
//     //     return nullptr;
//     // }
//     // std::vector<unsigned char> rgbaData(width * height * 4);
//     // for (int i = 0; i < width * height; ++i)
//     // {
//     //     rgbaData[i * 4] = data[i * 3];         // R
//     //     rgbaData[i * 4 + 1] = data[i * 3 + 1]; // G
//     //     rgbaData[i * 4 + 2] = data[i * 3 + 2]; // B
//     //     rgbaData[i * 4 + 3] = 255;             // A=255（完全不透明）
//     // }
//     // auto image = mImageManager->CreateUniqueTexture2D({static_cast<uint32_t>(width),
//     static_cast<uint32_t>(height)},
//     //                                                   vk::Format::eR8G8B8A8Srgb, 1, rgbaData.data());

//     // auto image1 = image->GetImage();
//     // image.release();
//     // stbi_image_free(data);
//     // // 创建图像视图
//     // auto imageView1 = mImageManager->CreateImageView(image1, vk::Format::eR8G8B8A8Srgb, vk::ComponentMapping{},
//     //                                                  {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
//     // if (!imageView1)
//     // {
//     //     LogE("Failed to create image view: {}", vk::to_string(vk::Result::eErrorUnknown));
//     //     return nullptr;
//     // }
//     // vk::ImageView imageView = imageView1.release();
//     // // 创建采样器
//     // auto sampler = mSamplerManager->CreateUniqueSampler();
//     // if (!sampler)
//     // {
//     //     LogE("Failed to create sampler: {}", vk::to_string(vk::Result::eErrorUnknown));
//     //     return nullptr;
//     // }
//     // vk::Sampler sampler1 = sampler.release();
//     // ImageDescriptor albedo;
//     // albedo.imageView = imageView;
//     // albedo.sampler = sampler1;
//     // auto imageDescriptors = std::vector<ImageDescriptor>{albedo};
//     // mDescriptorManager->UpdateCombinedSamplerImageDescriptorSet(
//     //     imageDescriptors, 1, pbrMaterial->GetDescriptorSet()); // TODO: add image info
//     // return pbrMaterial;
// }

// std::shared_ptr<Material> MaterialManager::GetMaterial(MaterialType type)
// {
//     auto it = mMaterials.find(type);
//     if (it != mMaterials.end())
//     {
//         auto material = it->second.lock();
//         if (material)
//         {
//             return material;
//         }
//     }

//     if (type == MaterialType::Default)
//     {
//         auto defaultMaterial = CreateDefaultMaterial();
//         mMaterials[type] = defaultMaterial;
//         return defaultMaterial;
//     }
//     else if (type == MaterialType::PBR)
//     {
//         auto pbrMaterial = CreatePBRMaterial();
//         mMaterials[type] = pbrMaterial;
//         return pbrMaterial;
//     }
//     return nullptr;
// }
// } // namespace MEngine