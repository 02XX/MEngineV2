#pragma once
#include "MEngine.hpp"
#include "SDL3/SDL_events.h"

class IScene
{
  public:
    IScene() = default;
    virtual ~IScene() = default;
    /**
     * @brief Called when the scene is created
     *
     */
    virtual void OnCreate() = 0;
    /**
     * @brief Handles the events of the sceneo
     *
     * @param event
     */
    virtual void HandleEvent(const SDL_Event &event) = 0;
    /**
     * @brief Called when the scene is updated, called every frame. processing game logic
     *
     */
    virtual void Update() = 0;
    /**
     * @brief Called after the scene is updated, called every frame.
     *
     */
    virtual void LateUpdate() = 0;
    /**
     * @brief Called when the scene is drawn, called every frame.
     *
     */
    virtual void Draw() = 0;
    /**
     * @brief Called when the scene is destroyed
     *
     */
    virtual void OnDestroy() = 0;
};