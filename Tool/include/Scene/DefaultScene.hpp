#include "BasicGeometry/BasicGeometryEntityManager.hpp"
#include "IScene.hpp"
#include "MEngine.hpp"
#include "System/RenderSystem.hpp"
#include "System/UISystem.hpp"
#include "entt/entt.hpp"
namespace MEngine
{
class DefaultScene : public IScene
{
  public:
    DefaultScene() = default;

    void OnCreate() override;
    void HandleEvent(const SDL_Event &event) override;
    void Update() override;
    void LateUpdate() override;
    void Draw() override;
    void OnDestroy() override;

  private:
    std::shared_ptr<entt::registry> mRegistry;
    // std::unique_ptr<RenderSystem> mRenderSystem;
    // std::unique_ptr<UISystem> mUISystem;
    // std::unique_ptr<BasicGeometryEntityManager> mBasicGeometryEntityManager;
};
} // namespace MEngine