#pragma once

#include "poorcraft/core/Event.h"
#include <functional>
#include <unordered_map>
#include <vector>
#include <queue>
#include <memory>
#include <mutex>

namespace PoorCraft {

using EventListener = std::function<void(Event&)>;

class EventBus {
public:
    static EventBus& getInstance();

    // Subscribe to specific event type, returns subscription ID
    size_t subscribe(EventType type, EventListener listener);

    // Unsubscribe using subscription ID
    void unsubscribe(size_t subscriptionId);

    // Publish event immediately to all listeners
    void publish(Event& event);

    // Queue event for deferred processing
    void queueEvent(std::unique_ptr<Event> event);

    // Process all queued events
    void processEvents();

    // Clear all subscriptions and queued events
    void clear();

    // Non-copyable, non-movable
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;
    EventBus(EventBus&&) = delete;
    EventBus& operator=(EventBus&&) = delete;

private:
    EventBus() = default;
    ~EventBus() = default;

    struct Subscription {
        size_t id;
        EventListener listener;
    };

    std::mutex m_Mutex;
    std::unordered_map<EventType, std::vector<Subscription>> m_Listeners;
    std::vector<Subscription> m_WildcardListeners; // Listen to all events
    std::queue<std::unique_ptr<Event>> m_EventQueue;
    size_t m_NextSubscriptionId = 1;
};

} // namespace PoorCraft
