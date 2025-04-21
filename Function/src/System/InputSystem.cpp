#include "System/InputSystem.hpp"

namespace MEngine
{
InputSystem::InputSystem(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                         std::shared_ptr<IConfigure> configure, std::shared_ptr<entt::registry> registry,
                         std::shared_ptr<IWindow> window)
    : System(logger, context, configure, registry), mWindow(window)
{
}
void InputSystem::InputSystem::Init()
{
    mWindow->SetEventCallback([this](const void *event) {
        auto sdlEvent = static_cast<const SDL_Event *>(event);
        HandleSDLEvent(sdlEvent);
    });
    mLogger->Info("InputSystem initialized.");
}

void InputSystem::Tick(float deltaTime)
{
    auto entities = mRegistry->view<InputComponent>();
    while (!mSDLEvents.IsEmpty())
    {
        for (auto entity : entities)
        {
            auto &inputComponent = entities.get<InputComponent>(entity);
            if (inputComponent.eventCallback)
            {
                SDL_Event frontEvent = mSDLEvents.Front();
                inputComponent.eventCallback(&frontEvent, deltaTime);
            }
        }
        mSDLEvents.Pop();
    }
}
void InputSystem::Shutdown()
{
    mLogger->Info("InputSystem shutdown.");
}
void InputSystem::HandleSDLEvent(const SDL_Event *event)
{
    mSDLEvents.PushOverwrite(*event);
    // mLogger->Trace("HandleSDLEvent: type = {}, action = {}, key = {}, button = {}, position = ({}, {})",
    //                static_cast<int>(event->type), 0, static_cast<int>(event->key.key),
    //                static_cast<int>(event->button.button), event->motion.x, event->motion.y);
}
} // namespace MEngine