#pragma once
#include "Component/InputComponent.hpp"
#include "RingBuffer.hpp"
#include "SDL3/SDL.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_filesystem.h"
#include "System/ISystem.hpp"
#include "System/System.hpp"
#include <memory>
#include <queue>
#include <unordered_map>

namespace MEngine
{
class InputSystem final : public System
{
  private:
    // DI
    std::shared_ptr<IWindow> mWindow;

  private:
    RingBuffer<InputEvent> mInputEvents{1000};
    RingBuffer<SDL_Event> mSDLEvents{1000};
    std::unordered_map<SDL_Keycode, InputKey> mKeyMap{{SDLK_A, InputKey::A},
                                                      {SDLK_B, InputKey::B},
                                                      {SDLK_C, InputKey::C},
                                                      {SDLK_D, InputKey::D},
                                                      {SDLK_E, InputKey::E},
                                                      {SDLK_F, InputKey::F},
                                                      {SDLK_G, InputKey::G},
                                                      {SDLK_H, InputKey::H},
                                                      {SDLK_I, InputKey::I},
                                                      {SDLK_J, InputKey::J},
                                                      {SDLK_K, InputKey::K},
                                                      {SDLK_L, InputKey::L},
                                                      {SDLK_M, InputKey::M},
                                                      {SDLK_N, InputKey::N},
                                                      {SDLK_O, InputKey::O},
                                                      {SDLK_P, InputKey::P},
                                                      {SDLK_Q, InputKey::Q},
                                                      {SDLK_R, InputKey::R},
                                                      {SDLK_S, InputKey::S},
                                                      {SDLK_T, InputKey::T},
                                                      {SDLK_U, InputKey::U},
                                                      {SDLK_V, InputKey::V},
                                                      {SDLK_W, InputKey::W},
                                                      {SDLK_X, InputKey::X},
                                                      {SDLK_Y, InputKey::Y},
                                                      {SDLK_Z, InputKey::Z},
                                                      {SDLK_0, InputKey::NUM_0},
                                                      {SDLK_1, InputKey::NUM_1},
                                                      {SDLK_2, InputKey::NUM_2},
                                                      {SDLK_3, InputKey::NUM_3},
                                                      {SDLK_4, InputKey::NUM_4},
                                                      {SDLK_5, InputKey::NUM_5},
                                                      {SDLK_6, InputKey::NUM_6},
                                                      {SDLK_7, InputKey::NUM_7},
                                                      {SDLK_8, InputKey::NUM_8},
                                                      {SDLK_9, InputKey::NUM_9},
                                                      {SDLK_SPACE, InputKey::SPACE},
                                                      {SDLK_RETURN, InputKey::ENTER},
                                                      {SDLK_ESCAPE, InputKey::ESCAPE},
                                                      {SDLK_TAB, InputKey::TAB},
                                                      {SDLK_BACKSPACE, InputKey::BACKSPACE},
                                                      {SDLK_INSERT, InputKey::INSERT},
                                                      {SDLK_DELETE, InputKey::DEL},
                                                      {SDLK_LEFT, InputKey::LEFT},
                                                      {SDLK_RIGHT, InputKey::RIGHT},
                                                      {SDLK_UP, InputKey::UP},
                                                      {SDLK_DOWN, InputKey::DOWN},
                                                      {SDLK_PAGEUP, InputKey::PAGE_UP},
                                                      {SDLK_PAGEDOWN, InputKey::PAGE_DOWN},
                                                      {SDLK_HOME, InputKey::HOME},
                                                      {SDLK_END, InputKey::END},
                                                      {SDLK_CAPSLOCK, InputKey::CAPS_LOCK},
                                                      {SDLK_SCROLLLOCK, InputKey::SCROLL_LOCK},
                                                      {SDLK_NUMLOCKCLEAR, InputKey::NUM_LOCK},
                                                      {SDLK_PRINTSCREEN, InputKey::PRINT_SCREEN},
                                                      {SDLK_PAUSE, InputKey::PAUSE},
                                                      {SDLK_F1, InputKey::F1},
                                                      {SDLK_F2, InputKey::F2},
                                                      {SDLK_F3, InputKey::F3},
                                                      {SDLK_F4, InputKey::F4},
                                                      {SDLK_F5, InputKey::F5},
                                                      {SDLK_F6, InputKey::F6},
                                                      {SDLK_F7, InputKey::F7},
                                                      {SDLK_F8, InputKey::F8},
                                                      {SDLK_F9, InputKey::F9},
                                                      {SDLK_F10, InputKey::F10},
                                                      {SDLK_F11, InputKey::F11},
                                                      {SDLK_F12, InputKey::F12},
                                                      {SDLK_LSHIFT, InputKey::LEFT_SHIFT},
                                                      {SDLK_RSHIFT, InputKey::RIGHT_SHIFT},
                                                      {SDLK_LCTRL, InputKey::LEFT_CTRL},
                                                      {SDLK_RCTRL, InputKey::RIGHT_CTRL},
                                                      {SDLK_LALT, InputKey::LEFT_ALT},
                                                      {SDLK_RALT, InputKey::RIGHT_ALT},
                                                      {SDLK_LGUI, InputKey::LEFT_META},
                                                      {SDLK_RGUI, InputKey::RIGHT_META}};

    std::unordered_map<SDL_Keymod, InputModifier> mModifierMap{{SDL_KMOD_NONE, InputModifier::NONE},
                                                               {SDL_KMOD_LSHIFT, InputModifier::LEFT_SHIFT},
                                                               {SDL_KMOD_RSHIFT, InputModifier::RIGHT_SHIFT},
                                                               {SDL_KMOD_LCTRL, InputModifier::LEFT_CTRL},
                                                               {SDL_KMOD_RCTRL, InputModifier::RIGHT_CTRL},
                                                               {SDL_KMOD_LALT, InputModifier::LEFT_ALT},
                                                               {SDL_KMOD_RALT, InputModifier::RIGHT_ALT},
                                                               {SDL_KMOD_LGUI, InputModifier::LEFT_META},
                                                               {SDL_KMOD_RGUI, InputModifier::RIGHT_META},
                                                               {SDL_KMOD_NUM, InputModifier::NUM_LOCK},
                                                               {SDL_KMOD_CAPS, InputModifier::CAPS_LOCK},
                                                               {SDL_KMOD_SCROLL, InputModifier::SCROLL_LOCK},
                                                               {SDL_KMOD_CTRL, InputModifier::CTRL},
                                                               {SDL_KMOD_SHIFT, InputModifier::SHIFT},
                                                               {SDL_KMOD_ALT, InputModifier::ALT},
                                                               {SDL_KMOD_GUI, InputModifier::META}};

  public:
    InputSystem(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                std::shared_ptr<IConfigure> configure, std::shared_ptr<entt::registry> registry,
                std::shared_ptr<IWindow> window);

  public:
    void Init() override;
    void HandleSDLEvent(const SDL_Event *event);
    void Tick(float deltaTime) override;
    void Shutdown() override;
};
} // namespace MEngine