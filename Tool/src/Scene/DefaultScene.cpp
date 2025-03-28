#include "Scene/DefaultScene.hpp"

namespace MEngine
{
void DefaultScene::OnCreate()
{
    mRegistry = std::make_shared<entt::registry>();
    mBasicGeometryEntityManager = std::make_unique<BasicGeometryEntityManager>(mRegistry);
    mRenderSystem = std::make_unique<RenderSystem>(mRegistry);
    // mBasicGeometryEntityManager->CreateSphere();
    auto cube = mBasicGeometryEntityManager->CreateCube();
}
void DefaultScene::HandleEvent(const SDL_Event &event)
{
}
void DefaultScene::Update()
{
}
void DefaultScene::LateUpdate()
{
}
void DefaultScene::Draw()
{
    mRenderSystem->Tick();
}
void DefaultScene::OnDestroy()
{
}
} // namespace MEngine