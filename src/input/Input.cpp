#include "poorcraft/input/Input.h"
#include "poorcraft/events/InputEvent.h"
#include "poorcraft/window/Window.h"
#include "poorcraft/core/Logger.h"

#include <GLFW/glfw3.h>

namespace PoorCraft {

Input& Input::getInstance() {
    static Input instance;
    return instance;
}

void Input::setWindow(Window* window) {
    m_Window = window;
    PC_INFO("[Input] Input system initialized");
}

void Input::onEvent(Event& event) {
    EventDispatcher dispatcher(event);
    
    dispatcher.dispatch<KeyPressEvent>([this](KeyPressEvent& e) {
        int key = e.getKeyCode();
        if (key >= 0 && key < MAX_KEYS) {
            if (!m_KeysCurrent[key]) {
                m_KeysJustPressed[key] = true;
            }
            m_KeysCurrent[key] = true;
        }
        return false; // Don't mark as handled, allow other systems to process
    });
    
    dispatcher.dispatch<KeyReleaseEvent>([this](KeyReleaseEvent& e) {
        int key = e.getKeyCode();
        if (key >= 0 && key < MAX_KEYS) {
            m_KeysCurrent[key] = false;
            m_KeysJustReleased[key] = true;
        }
        return false;
    });
    
    dispatcher.dispatch<MouseButtonPressEvent>([this](MouseButtonPressEvent& e) {
        int button = e.getButton();
        if (button >= 0 && button < MAX_MOUSE_BUTTONS) {
            if (!m_MouseButtonsCurrent[button]) {
                m_MouseButtonsJustPressed[button] = true;
            }
            m_MouseButtonsCurrent[button] = true;
        }
        return false;
    });
    
    dispatcher.dispatch<MouseButtonReleaseEvent>([this](MouseButtonReleaseEvent& e) {
        int button = e.getButton();
        if (button >= 0 && button < MAX_MOUSE_BUTTONS) {
            m_MouseButtonsCurrent[button] = false;
            m_MouseButtonsJustReleased[button] = true;
        }
        return false;
    });
    
    dispatcher.dispatch<MouseMoveEvent>([this](MouseMoveEvent& e) {
        m_MousePosition.x = static_cast<float>(e.getX());
        m_MousePosition.y = static_cast<float>(e.getY());
        m_MouseDelta.x = static_cast<float>(e.getDeltaX());
        m_MouseDelta.y = static_cast<float>(e.getDeltaY());
        return false;
    });
    
    dispatcher.dispatch<MouseScrollEvent>([this](MouseScrollEvent& e) {
        m_MouseScroll.x = static_cast<float>(e.getXOffset());
        m_MouseScroll.y = static_cast<float>(e.getYOffset());
        return false;
    });
}

void Input::update() {
    // Copy current to previous
    m_KeysPrevious = m_KeysCurrent;
    m_MouseButtonsPrevious = m_MouseButtonsCurrent;
    
    // Clear just pressed/released flags
    m_KeysJustPressed.fill(false);
    m_KeysJustReleased.fill(false);
    m_MouseButtonsJustPressed.fill(false);
    m_MouseButtonsJustReleased.fill(false);
    
    // Reset deltas
    m_MouseDelta.x = 0.0f;
    m_MouseDelta.y = 0.0f;
    m_MouseScroll.x = 0.0f;
    m_MouseScroll.y = 0.0f;
    
    // Poll gamepad states
    for (int i = 0; i < MAX_GAMEPADS; ++i) {
        bool wasConnected = m_Gamepads[i].connected;
        m_Gamepads[i].connected = glfwJoystickPresent(i) && glfwJoystickIsGamepad(i);
        
        if (m_Gamepads[i].connected) {
            if (!wasConnected) {
                PC_INFO("[Input] Gamepad " + std::to_string(i) + " connected: " + getGamepadName(i));
            }
            
            GLFWgamepadstate state;
            if (glfwGetGamepadState(i, &state)) {
                for (int j = 0; j < 15; ++j) {
                    m_Gamepads[i].buttons[j] = state.buttons[j] == GLFW_PRESS;
                }
                for (int j = 0; j < 6; ++j) {
                    m_Gamepads[i].axes[j] = state.axes[j];
                }
            }
        } else if (wasConnected) {
            PC_INFO("[Input] Gamepad " + std::to_string(i) + " disconnected");
        }
    }
}

bool Input::isKeyPressed(int keyCode) const {
    if (keyCode < 0 || keyCode >= MAX_KEYS) return false;
    return m_KeysCurrent[keyCode];
}

bool Input::isKeyReleased(int keyCode) const {
    if (keyCode < 0 || keyCode >= MAX_KEYS) return false;
    return !m_KeysCurrent[keyCode];
}

bool Input::isKeyHeld(int keyCode) const {
    if (keyCode < 0 || keyCode >= MAX_KEYS) return false;
    return m_KeysCurrent[keyCode] && m_KeysPrevious[keyCode];
}

bool Input::wasKeyJustPressed(int keyCode) const {
    if (keyCode < 0 || keyCode >= MAX_KEYS) return false;
    return m_KeysJustPressed[keyCode];
}

bool Input::wasKeyJustReleased(int keyCode) const {
    if (keyCode < 0 || keyCode >= MAX_KEYS) return false;
    return m_KeysJustReleased[keyCode];
}

bool Input::isMouseButtonPressed(int button) const {
    if (button < 0 || button >= MAX_MOUSE_BUTTONS) return false;
    return m_MouseButtonsCurrent[button];
}

bool Input::isMouseButtonReleased(int button) const {
    if (button < 0 || button >= MAX_MOUSE_BUTTONS) return false;
    return !m_MouseButtonsCurrent[button];
}

bool Input::wasMouseButtonJustPressed(int button) const {
    if (button < 0 || button >= MAX_MOUSE_BUTTONS) return false;
    return m_MouseButtonsJustPressed[button];
}

bool Input::wasMouseButtonJustReleased(int button) const {
    if (button < 0 || button >= MAX_MOUSE_BUTTONS) return false;
    return m_MouseButtonsJustReleased[button];
}

Vec2 Input::getMousePosition() const {
    return m_MousePosition;
}

Vec2 Input::getMouseDelta() const {
    return m_MouseDelta;
}

Vec2 Input::getMouseScroll() const {
    return m_MouseScroll;
}

void Input::setCursorMode(CursorMode mode) {
    if (!m_Window) return;
    
    GLFWwindow* window = m_Window->getNativeWindow();
    if (!window) return;
    
    int glfwMode;
    switch (mode) {
        case CursorMode::Normal:
            glfwMode = GLFW_CURSOR_NORMAL;
            break;
        case CursorMode::Hidden:
            glfwMode = GLFW_CURSOR_HIDDEN;
            break;
        case CursorMode::Disabled:
            glfwMode = GLFW_CURSOR_DISABLED;
            break;
        default:
            glfwMode = GLFW_CURSOR_NORMAL;
    }
    
    glfwSetInputMode(window, GLFW_CURSOR, glfwMode);
}

bool Input::isGamepadConnected(int gamepadId) const {
    if (gamepadId < 0 || gamepadId >= MAX_GAMEPADS) return false;
    return m_Gamepads[gamepadId].connected;
}

bool Input::isGamepadButtonPressed(int gamepadId, int button) const {
    if (gamepadId < 0 || gamepadId >= MAX_GAMEPADS) return false;
    if (!m_Gamepads[gamepadId].connected) return false;
    if (button < 0 || button >= 15) return false;
    return m_Gamepads[gamepadId].buttons[button];
}

float Input::getGamepadAxis(int gamepadId, int axis) const {
    if (gamepadId < 0 || gamepadId >= MAX_GAMEPADS) return 0.0f;
    if (!m_Gamepads[gamepadId].connected) return 0.0f;
    if (axis < 0 || axis >= 6) return 0.0f;
    return m_Gamepads[gamepadId].axes[axis];
}

std::string Input::getGamepadName(int gamepadId) const {
    if (gamepadId < 0 || gamepadId >= MAX_GAMEPADS) return "";
    if (!m_Gamepads[gamepadId].connected) return "";
    
    const char* name = glfwGetGamepadName(gamepadId);
    return name ? name : "";
}

} // namespace PoorCraft
