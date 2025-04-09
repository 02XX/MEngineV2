// #include "Context.hpp"
// #include "DescriptorManager.hpp"
// #include "ImageManager.hpp"
// #include "MEngine.hpp"
// #include "Material.hpp"
// #include "NoCopyable.hpp"
// #include "SamplerManager.hpp"
// #include "stb_image.h"
// #include <cstdint>
// #include <memory>
// #include <unordered_map>
// namespace MEngine
// {
// enum class MaterialType
// {
//     PBR,
//     Translucency,
// };
// class MaterialManager : public NoCopyable
// {
//   private:
//     // DI
//     std::shared_ptr<ILogger> mLogger;
//     std::shared_ptr<Context> mContext;

//   private:
//     std::unordered_map<uint32_t, std::weak_ptr<Material>> mMaterials; // 材质缓存
//   public:
//     MaterialManager();
//     std::shared_ptr<Material> GetMaterial(MaterialType type);
// };
// } // namespace MEngine