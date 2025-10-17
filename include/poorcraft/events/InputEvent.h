#pragma once

#include "poorcraft/core/Event.h"
#include <sstream>

namespace PoorCraft {

// Base class for keyboard events
class KeyEvent : public Event {
public:
    int getKeyCode() const { return m_KeyCode; }

    EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

protected:
    KeyEvent(int keyCode) : m_KeyCode(keyCode) {}

    int m_KeyCode;
};

// Key press event
class KeyPressEvent : public KeyEvent {
public:
    KeyPressEvent(int keyCode, bool repeat)
        : KeyEvent(keyCode), m_Repeat(repeat) {}

    bool isRepeat() const { return m_Repeat; }

    EVENT_CLASS_TYPE(KeyPress)

    std::string toString() const override {
        std::stringstream ss;
        ss << "KeyPressEvent: Key=" << m_KeyCode << ", Repeat=" << (m_Repeat ? "true" : "false");
        return ss.str();
    }

private:
    bool m_Repeat;
};

// Key release event
class KeyReleaseEvent : public KeyEvent {
public:
    KeyReleaseEvent(int keyCode)
        : KeyEvent(keyCode) {}

    EVENT_CLASS_TYPE(KeyRelease)

    std::string toString() const override {
        std::stringstream ss;
        ss << "KeyReleaseEvent: Key=" << m_KeyCode;
        return ss.str();
    }
};

// Mouse move event
class MouseMoveEvent : public Event {
public:
    MouseMoveEvent(double x, double y, double deltaX, double deltaY)
        : m_X(x), m_Y(y), m_DeltaX(deltaX), m_DeltaY(deltaY) {}

    double getX() const { return m_X; }
    double getY() const { return m_Y; }
    double getDeltaX() const { return m_DeltaX; }
    double getDeltaY() const { return m_DeltaY; }

    EVENT_CLASS_TYPE(MouseMove)
    EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

    std::string toString() const override {
        std::stringstream ss;
        ss << "MouseMoveEvent: (" << m_X << ", " << m_Y << "), Delta=(" << m_DeltaX << ", " << m_DeltaY << ")";
        return ss.str();
    }

private:
    double m_X, m_Y;
    double m_DeltaX, m_DeltaY;
};

// Base class for mouse button events
class MouseButtonEvent : public Event {
public:
    int getButton() const { return m_Button; }

    EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

protected:
    MouseButtonEvent(int button) : m_Button(button) {}

    int m_Button;
};

// Mouse button press event
class MouseButtonPressEvent : public MouseButtonEvent {
public:
    MouseButtonPressEvent(int button)
        : MouseButtonEvent(button) {}

    EVENT_CLASS_TYPE(MouseButtonPress)

    std::string toString() const override {
        std::stringstream ss;
        ss << "MouseButtonPressEvent: Button=" << m_Button;
        return ss.str();
    }
};

// Mouse button release event
class MouseButtonReleaseEvent : public MouseButtonEvent {
public:
    MouseButtonReleaseEvent(int button)
        : MouseButtonEvent(button) {}

    EVENT_CLASS_TYPE(MouseButtonRelease)

    std::string toString() const override {
        std::stringstream ss;
        ss << "MouseButtonReleaseEvent: Button=" << m_Button;
        return ss.str();
    }
};

// Mouse scroll event
class MouseScrollEvent : public Event {
public:
    MouseScrollEvent(double xOffset, double yOffset)
        : m_XOffset(xOffset), m_YOffset(yOffset) {}

    double getXOffset() const { return m_XOffset; }
    double getYOffset() const { return m_YOffset; }

    EVENT_CLASS_TYPE(MouseScroll)
    EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

    std::string toString() const override {
        std::stringstream ss;
        ss << "MouseScrollEvent: (" << m_XOffset << ", " << m_YOffset << ")";
        return ss.str();
    }

private:
    double m_XOffset, m_YOffset;
};

// Gamepad button event
class GamepadButtonEvent : public Event {
public:
    GamepadButtonEvent(int gamepadId, int button, bool pressed)
        : m_GamepadId(gamepadId), m_Button(button), m_Pressed(pressed) {}

    int getGamepadId() const { return m_GamepadId; }
    int getButton() const { return m_Button; }
    bool isPressed() const { return m_Pressed; }

    EVENT_CLASS_TYPE(GamepadButton)
    EVENT_CLASS_CATEGORY(EventCategoryGamepad | EventCategoryInput)

    std::string toString() const override {
        std::stringstream ss;
        ss << "GamepadButtonEvent: Gamepad=" << m_GamepadId << ", Button=" << m_Button 
           << ", " << (m_Pressed ? "Pressed" : "Released");
        return ss.str();
    }

private:
    int m_GamepadId;
    int m_Button;
    bool m_Pressed;
};

// Gamepad axis event
class GamepadAxisEvent : public Event {
public:
    GamepadAxisEvent(int gamepadId, int axis, float value)
        : m_GamepadId(gamepadId), m_Axis(axis), m_Value(value) {}

    int getGamepadId() const { return m_GamepadId; }
    int getAxis() const { return m_Axis; }
    float getValue() const { return m_Value; }

    EVENT_CLASS_TYPE(GamepadAxis)
    EVENT_CLASS_CATEGORY(EventCategoryGamepad | EventCategoryInput)

    std::string toString() const override {
        std::stringstream ss;
        ss << "GamepadAxisEvent: Gamepad=" << m_GamepadId << ", Axis=" << m_Axis 
           << ", Value=" << m_Value;
        return ss.str();
    }

private:
    int m_GamepadId;
    int m_Axis;
    float m_Value;
};

} // namespace PoorCraft
