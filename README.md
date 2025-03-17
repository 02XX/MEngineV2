# MEngine

该引擎分为5层，由上到下分别是工具层（Tool）、功能层（Function）、资源层（Resource）、核心层（Core）、平台层（Platform）以及公用的第三方插件（3rdParty）

## ToDO List

- [ ] 平台层
  - [ ] Vulkan封装
- [ ] 核心层
- [ ] 资源层
- [ ] 功能层
- [ ] 工具层
  - [x] 基本表面
  - [ ] UI

## Feature

### RAII

| ​**资源类型**| ​**Vulkan 句柄类型**| ​**RAII 封装类**| ​**销毁函数**|
|---|---|---|---|
| 实例                        | `VkInstance`             | `UniqueInstance`       | `vkDestroyInstance`             |
| 物理设备                    | `VkPhysicalDevice`       | -                      | - (无需销毁)                    |
| 逻辑设备                    | `VkDevice`               | `UniqueDevice`         | `vkDestroyDevice`               |
| 队列                        | `VkQueue`                | -                      | - (无需销毁)                    |
| 命令池                      | `VkCommandPool`          | `UniqueCommandPool`    | `vkDestroyCommandPool`          |
| 命令缓冲                    | `VkCommandBuffer`        | `UniqueCommandBuffer`  | `vkFreeCommandBuffers`          |
| 图像                        | `VkImage`                | `UniqueImage`          | `vkDestroyImage`                |
| 图像视图                    | `VkImageView`            | `UniqueImageView`      | `vkDestroyImageView`            |
| 缓冲区                      | `VkBuffer`               | `UniqueBuffer`         | `vkDestroyBuffer`               |
| 缓冲区视图                  | `VkBufferView`           | `UniqueBufferView`     | `vkDestroyBufferView`           |
| 帧缓冲                      | `VkFramebuffer`          | `UniqueFramebuffer`    | `vkDestroyFramebuffer`          |
| 渲染通道                    | `VkRenderPass`           | `UniqueRenderPass`     | `vkDestroyRenderPass`           |
| 着色器模块                  | `VkShaderModule`         | `UniqueShaderModule`   | `vkDestroyShaderModule`         |
| 管道布局                    | `VkPipelineLayout`       | `UniquePipelineLayout` | `vkDestroyPipelineLayout`       |
| 图形/计算管道               | `VkPipeline`             | `UniquePipeline`       | `vkDestroyPipeline`             |
| 描述符池                    | `VkDescriptorPool`       | `UniqueDescriptorPool` | `vkDestroyDescriptorPool`       |
| 描述符集布局                | `VkDescriptorSetLayout`  | `UniqueDescriptorSetLayout` | `vkDestroyDescriptorSetLayout` |
| 描述符集                    | `VkDescriptorSet`        | -                      | - (通过池释放)                  |
| 采样器                      | `VkSampler`              | `UniqueSampler`        | `vkDestroySampler`              |
| 信号量                      | `VkSemaphore`            | `UniqueSemaphore`      | `vkDestroySemaphore`            |
| 栅栏                        | `VkFence`                | `UniqueFence`          | `vkDestroyFence`                |
| 事件                        | `VkEvent`                | `UniqueEvent`          | `vkDestroyEvent`                |
| 查询池                      | `VkQueryPool`            | `UniqueQueryPool`      | `vkDestroyQueryPool`            |
| 交换链                      | `VkSwapchainKHR`         | `UniqueSwapchainKHR`   | `vkDestroySwapchainKHR`         |
| 表面                        | `VkSurfaceKHR`           | `UniqueSurfaceKHR`     | `vkDestroySurfaceKHR`           |
| 调试报告回调                | `VkDebugReportCallbackEXT` | `UniqueDebugReportCallbackEXT` | `vkDestroyDebugReportCallbackEXT` |
| 调试工具回调                | `VkDebugUtilsMessengerEXT` | `UniqueDebugUtilsMessengerEXT` | `vkDestroyDebugUtilsMessengerEXT` |
| 加速结构                    | `VkAccelerationStructureKHR` | `UniqueAccelerationStructureKHR` | `vkDestroyAccelerationStructureKHR` |
| 光线追踪管道                | `VkRayTracingPipelineKHR` | `UniqueRayTracingPipelineKHR` | `vkDestroyPipeline`             |
| 光线追踪着色器绑定表        | `VkShaderBindingTableKHR` | `UniqueShaderBindingTableKHR` | - (通过管道释放)                |
| 视频会话                    | `VkVideoSessionKHR`      | `UniqueVideoSessionKHR` | `vkDestroyVideoSessionKHR`      |
| 视频会话参数                | `VkVideoSessionParametersKHR` | `UniqueVideoSessionParametersKHR` | `vkDestroyVideoSessionParametersKHR` |

## Platform Layer

平台层使引擎能够兼容各种硬件平台，实现引擎的平台无关性。

## Core

## Resource

## Function Layer

## Tool
