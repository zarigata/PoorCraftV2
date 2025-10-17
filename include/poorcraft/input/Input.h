#pragma once

#include "poorcraft/core/Event.h"
#include <array>
#include <string>

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>

// Forward declare GLFWwindow
struct GLFWwindow;
struct GLFWgamepadstate;

namespace PoorCraft {

class Window;

struct Vec2 {
    float x, y;
    Vec2() : x(0), y(0) {}
    Vec2(float x, float y) : x(x), y(y) {}
};

enum class CursorMode {
    Normal,
    Hidden,
    Disabled
};

class Input {
public:
    static Input& getInstance();

    // Keyboard
    bool isKeyPressed(int keyCode) const;
    bool isKeyReleased(int keyCode) const;
    bool isKeyHeld(int keyCode) const;
    bool wasKeyJustPressed(int keyCode) const;
    bool wasKeyJustReleased(int keyCode) const;

    // Mouse
    bool isMouseButtonPressed(int button) const;
    bool isMouseButtonReleased(int button) const;
    bool wasMouseButtonJustPressed(int button) const;
    bool wasMouseButtonJustReleased(int button) const;
    Vec2 getMousePosition() const;
    Vec2 getMouseDelta() const;
    Vec2 getMouseScroll() const;
    void setCursorMode(CursorMode mode);

    // Gamepad
    bool isGamepadConnected(int gamepadId) const;
    bool isGamepadButtonPressed(int gamepadId, int button) const;
    float getGamepadAxis(int gamepadId, int axis) const;
    std::string getGamepadName(int gamepadId) const;

    // System
    void update();
    void onEvent(Event& event);
    void setWindow(Window* window);

    // Non-copyable, non-movable
    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;
    Input(Input&&) = delete;
    Input& operator=(Input&&) = delete;

private:
    Input() = default;
    ~Input() = default;

    static constexpr int MAX_KEYS = GLFW_KEY_LAST + 1;
    static constexpr int MAX_MOUSE_BUTTONS = GLFW_MOUSE_BUTTON_LAST + 1;
    static constexpr int MAX_GAMEPADS = 16;

    std::array<bool, MAX_KEYS> m_KeysCurrent{};
    std::array<bool, MAX_KEYS> m_KeysPrevious{};
    std::array<bool, MAX_KEYS> m_KeysJustPressed{};
    std::array<bool, MAX_KEYS> m_KeysJustReleased{};

    std::array<bool, MAX_MOUSE_BUTTONS> m_MouseButtonsCurrent{};
    std::array<bool, MAX_MOUSE_BUTTONS> m_MouseButtonsPrevious{};
    std::array<bool, MAX_MOUSE_BUTTONS> m_MouseButtonsJustPressed{};
    std::array<bool, MAX_MOUSE_BUTTONS> m_MouseButtonsJustReleased{};

    Vec2 m_MousePosition;
    Vec2 m_MouseDelta;
    Vec2 m_MouseScroll;

    struct GamepadState {
        bool connected = false;
        std::array<bool, 15> buttons{};
        std::array<float, 6> axes{};
    };
    std::array<GamepadState, MAX_GAMEPADS> m_Gamepads;

    Window* m_Window = nullptr;
};

} // namespace PoorCraft
