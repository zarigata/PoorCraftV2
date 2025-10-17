#pragma once

#include "poorcraft/core/Event.h"
#include <sstream>

namespace PoorCraft {

// Window close event
class WindowCloseEvent : public Event {
public:
    WindowCloseEvent() = default;

    EVENT_CLASS_TYPE(WindowClose)
    EVENT_CLASS_CATEGORY(EventCategoryWindow)

    std::string toString() const override {
        return "WindowCloseEvent";
    }
};

// Window resize event
class WindowResizeEvent : public Event {
public:
    WindowResizeEvent(uint32_t width, uint32_t height)
        : m_Width(width), m_Height(height) {}

    uint32_t getWidth() const { return m_Width; }
    uint32_t getHeight() const { return m_Height; }

    EVENT_CLASS_TYPE(WindowResize)
    EVENT_CLASS_CATEGORY(EventCategoryWindow)

    std::string toString() const override {
        std::stringstream ss;
        ss << "WindowResizeEvent: " << m_Width << "x" << m_Height;
        return ss.str();
    }

private:
    uint32_t m_Width, m_Height;
};

// Window focus event
class WindowFocusEvent : public Event {
public:
    WindowFocusEvent(bool focused)
        : m_Focused(focused) {}

    bool isFocused() const { return m_Focused; }

    EVENT_CLASS_TYPE(WindowFocus)
    EVENT_CLASS_CATEGORY(EventCategoryWindow)

    std::string toString() const override {
        std::stringstream ss;
        ss << "WindowFocusEvent: " << (m_Focused ? "Focused" : "Unfocused");
        return ss.str();
    }

private:
    bool m_Focused;
};

// Window minimize event
class WindowMinimizeEvent : public Event {
public:
    WindowMinimizeEvent(bool minimized)
        : m_Minimized(minimized) {}

    bool isMinimized() const { return m_Minimized; }

    EVENT_CLASS_TYPE(WindowMinimize)
    EVENT_CLASS_CATEGORY(EventCategoryWindow)

    std::string toString() const override {
        std::stringstream ss;
        ss << "WindowMinimizeEvent: " << (m_Minimized ? "Minimized" : "Restored");
        return ss.str();
    }

private:
    bool m_Minimized;
};

// Window move event
class WindowMoveEvent : public Event {
public:
    WindowMoveEvent(int x, int y)
        : m_X(x), m_Y(y) {}

    int getX() const { return m_X; }
    int getY() const { return m_Y; }

    EVENT_CLASS_TYPE(WindowMove)
    EVENT_CLASS_CATEGORY(EventCategoryWindow)

    std::string toString() const override {
        std::stringstream ss;
        ss << "WindowMoveEvent: (" << m_X << ", " << m_Y << ")";
        return ss.str();
    }

private:
    int m_X, m_Y;
};

} // namespace PoorCraft
