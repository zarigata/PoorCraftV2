#pragma once

#include <string>
#include <functional>
#include <memory>

namespace PoorCraft {

// Event types for all engine events
enum class EventType {
    None = 0,
    WindowClose, WindowResize, WindowFocus, WindowMinimize, WindowMove,
    KeyPress, KeyRelease,
    MouseMove, MouseButtonPress, MouseButtonRelease, MouseScroll,
    GamepadButton, GamepadAxis,
    PlayerJoined, PlayerLeft, ConnectionEstablished, ConnectionLost,
    ChunkReceived, ServerStarted, ServerStopped,
    ModLoaded, ModUnloaded, ModReloaded,
    BlockPlaced, BlockBroken,
    EntitySpawned, EntityDestroyed,
    PlayerInteract,
    ChunkGenerated
};

// Event categories for filtering
enum EventCategory {
    None = 0,
    EventCategoryWindow = 1 << 0,
    EventCategoryInput = 1 << 1,
    EventCategoryKeyboard = 1 << 2,
    EventCategoryMouse = 1 << 3,
    EventCategoryGamepad = 1 << 4,
    EventCategoryNetwork = 1 << 5,
    EventCategoryMod = 1 << 6
};

// Macros for event class boilerplate
#define EVENT_CLASS_TYPE(type) \
    static EventType getStaticType() { return EventType::type; } \
    virtual EventType getType() const override { return getStaticType(); } \
    virtual const char* getName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) \
    virtual int getCategoryFlags() const override { return category; }

// Abstract base class for all events
class Event {
public:
    virtual ~Event() = default;

    virtual EventType getType() const = 0;
    virtual const char* getName() const = 0;
    virtual int getCategoryFlags() const = 0;
    virtual std::string toString() const { return getName(); }

    bool isInCategory(EventCategory category) const {
        return getCategoryFlags() & category;
    }

    bool isHandled() const { return m_Handled; }
    void setHandled(bool handled = true) { m_Handled = handled; }

    // Non-copyable but movable
    Event(const Event&) = delete;
    Event& operator=(const Event&) = delete;
    Event(Event&&) = default;
    Event& operator=(Event&&) = default;

protected:
    Event() = default;
    bool m_Handled = false;
};

// Event dispatcher for type-safe event handling
class EventDispatcher {
public:
    EventDispatcher(Event& event) : m_Event(event) {}

    template<typename T, typename F>
    bool dispatch(const F& func) {
        if (m_Event.getType() == T::getStaticType()) {
            m_Event.setHandled(func(static_cast<T&>(m_Event)));
            return true;
        }
        return false;
    }

private:
    Event& m_Event;
};

} // namespace PoorCraft
