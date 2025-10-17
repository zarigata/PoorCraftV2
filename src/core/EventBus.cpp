#include "poorcraft/core/EventBus.h"
#include "poorcraft/core/Logger.h"
#include <algorithm>
#include <vector>

namespace PoorCraft {

EventBus& EventBus::getInstance() {
    static EventBus instance;
    return instance;
}

size_t EventBus::subscribe(EventType type, EventListener listener) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    size_t id = m_NextSubscriptionId++;
    Subscription sub{id, std::move(listener)};

    if (type == EventType::None) {
        // Wildcard subscription - listen to all events
        m_WildcardListeners.push_back(std::move(sub));
        PC_TRACE("[EventBus] Wildcard subscription added with ID: " + std::to_string(id));
    } else {
        m_Listeners[type].push_back(std::move(sub));
        PC_TRACE("[EventBus] Subscription added for event type with ID: " + std::to_string(id));
    }

    return id;
}

void EventBus::unsubscribe(size_t subscriptionId) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    // Remove from wildcard listeners
    auto wildcardIt = std::remove_if(m_WildcardListeners.begin(), m_WildcardListeners.end(),
        [subscriptionId](const Subscription& sub) { return sub.id == subscriptionId; });

    if (wildcardIt != m_WildcardListeners.end()) {
        m_WildcardListeners.erase(wildcardIt, m_WildcardListeners.end());
        PC_TRACE("[EventBus] Unsubscribed ID: " + std::to_string(subscriptionId));
        return;
    }

    // Remove from specific event type listeners
    for (auto& [type, listeners] : m_Listeners) {
        auto it = std::remove_if(listeners.begin(), listeners.end(),
            [subscriptionId](const Subscription& sub) { return sub.id == subscriptionId; });

        if (it != listeners.end()) {
            listeners.erase(it, listeners.end());
            PC_TRACE("[EventBus] Unsubscribed ID: " + std::to_string(subscriptionId));
            return;
        }
    }
}

void EventBus::publish(Event& event) {
    std::vector<Subscription> wildcardListeners;
    std::vector<Subscription> specificListeners;

    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        wildcardListeners = m_WildcardListeners;

        auto it = m_Listeners.find(event.getType());
        if (it != m_Listeners.end()) {
            specificListeners = it->second;
        }
    }

    // Notify wildcard listeners without holding the mutex
    for (auto& sub : wildcardListeners) {
        if (event.isHandled()) break;
        try {
            sub.listener(event);
        } catch (const std::exception& e) {
            PC_ERROR(std::string("[EventBus] Exception in wildcard event listener: ") + e.what());
        }
    }

    // Notify specific event type listeners
    for (auto& sub : specificListeners) {
        if (event.isHandled()) break;
        try {
            sub.listener(event);
        } catch (const std::exception& e) {
            PC_ERROR(std::string("[EventBus] Exception in event listener: ") + e.what());
        }
    }
}

void EventBus::queueEvent(std::unique_ptr<Event> event) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_EventQueue.push(std::move(event));
}

void EventBus::processEvents() {
    // Move events to temporary queue to avoid holding lock during processing
    std::queue<std::unique_ptr<Event>> eventsToProcess;
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        eventsToProcess.swap(m_EventQueue);
    }
    
    // Process events without holding the lock
    while (!eventsToProcess.empty()) {
        auto& event = eventsToProcess.front();
        publish(*event);
        eventsToProcess.pop();
    }
}

void EventBus::clear() {
    std::lock_guard<std::mutex> lock(m_Mutex);

    m_Listeners.clear();
    m_WildcardListeners.clear();

    // Clear event queue
    while (!m_EventQueue.empty()) {
        m_EventQueue.pop();
    }

    PC_INFO("[EventBus] Cleared all subscriptions and queued events");
}

} // namespace PoorCraft
