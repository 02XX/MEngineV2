#pragma once
#include "Component/Interface/IComponent.hpp"
#include "MEngine.hpp"
#include "Math.hpp"
#include "RingBuffer.hpp"
#include "SDL3/SDL_events.h"
#include <functional>
#include <queue>
#include <unordered_map>
#include <vulkan/vulkan.h>

namespace MEngine
{
enum class InputType : uint8_t
{
    KEYBOARD = 0,
    MOUSE = 1,
    CONTROLLER = 2,
    TOUCH = 3,
    NONE = 4
};

enum class InputAction : uint8_t
{
    PRESS = 0,
    RELEASE = 1,
    HOLD = 2,
    NONE = 3
};

enum class InputButton : uint8_t
{
    LEFT = 0,
    RIGHT = 1,
    MIDDLE = 2,
    BACK = 3,
    FORWARD = 4,
    SCROLL_UP = 5,
    SCROLL_DOWN = 6,
    NONE = 7
};

enum class InputAxis : uint8_t
{
    X = 0,
    Y = 1,
    Z = 2,
    NONE = 3
};

enum class InputKey : uint8_t
{
    A = 0,
    B = 1,
    C = 2,
    D = 3,
    E = 4,
    F = 5,
    G = 6,
    H = 7,
    I = 8,
    J = 9,
    K = 10,
    L = 11,
    M = 12,
    N = 13,
    O = 14,
    P = 15,
    Q = 16,
    R = 17,
    S = 18,
    T = 19,
    U = 20,
    V = 21,
    W = 22,
    X = 23,
    Y = 24,
    Z = 25,
    NUM_0 = 26,
    NUM_1 = 27,
    NUM_2 = 28,
    NUM_3 = 29,
    NUM_4 = 30,
    NUM_5 = 31,
    NUM_6 = 32,
    NUM_7 = 33,
    NUM_8 = 34,
    NUM_9 = 35,
    ESCAPE = 36,
    ENTER = 37,
    TAB = 38,
    BACKSPACE = 39,
    INSERT = 40,
    DEL = 41,
    RIGHT = 42,
    LEFT = 43,
    DOWN = 44,
    UP = 45,
    PAGE_UP = 46,
    PAGE_DOWN = 47,
    HOME = 48,
    END = 49,
    CAPS_LOCK = 50,
    SCROLL_LOCK = 51,
    NUM_LOCK = 52,
    PRINT_SCREEN = 53,
    PAUSE = 54,
    F1 = 55,
    F2 = 56,
    F3 = 57,
    F4 = 58,
    F5 = 59,
    F6 = 60,
    F7 = 61,
    F8 = 62,
    F9 = 63,
    F10 = 64,
    F11 = 65,
    F12 = 66,
    LEFT_SHIFT = 67,
    LEFT_CTRL = 68,
    LEFT_ALT = 69,
    LEFT_META = 70,
    RIGHT_SHIFT = 71,
    RIGHT_CTRL = 72,
    RIGHT_ALT = 73,
    RIGHT_META = 74,
    SPACE = 75,
    NONE = 76,
};
enum class InputModifier : uint8_t
{
    LEFT_SHIFT,
    RIGHT_SHIFT,
    LEFT_CTRL,
    RIGHT_CTRL,
    LEFT_ALT,
    RIGHT_ALT,
    LEFT_META,
    RIGHT_META,
    NUM_LOCK,
    CAPS_LOCK,
    SCROLL_LOCK,
    CTRL,
    SHIFT,
    ALT,
    META,
    NONE
};

// 输入事件结构
struct InputEvent
{
    InputType type;
    InputAction action;
    union {
        InputKey key;
        InputButton button;
        InputAxis axis;
    };
    InputModifier modifier;
    glm::vec2 position; // 对于鼠标/触摸事件
    float value;        // 对于轴值或压力
    uint64_t timestamp;
};

// 输入组件
struct InputComponent : public IComponent<>
{
    std::function<void(const SDL_Event *, float deltaTime)> eventCallback = nullptr;
};
} // namespace MEngine