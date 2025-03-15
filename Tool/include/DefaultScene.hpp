#pragma once
#include "MEngine.hpp"
#include "Scene.hpp"

class MENGINE_API DefaultScene final : public IScene
{
  public:
    DefaultScene();

    void OnCreate() override;
    void HandleEvent(const SDL_Event &event) override;
    void Update() override;
    void LateUpdate() override;
    void Draw() override;
    void OnDestroy() override;
};
