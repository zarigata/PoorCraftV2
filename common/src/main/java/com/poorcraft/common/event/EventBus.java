package com.poorcraft.common.event;

import com.poorcraft.api.event.Event;
import com.poorcraft.api.event.Event.Cancellable;
import com.poorcraft.api.event.EventHandler;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.ConcurrentHashMap;

public class EventBus {
    private final Map<Class<? extends Event>, List<EventHandlerWrapper>> handlers = new ConcurrentHashMap<>();

    public void register(Object listener) {
        Objects.requireNonNull(listener, "listener");
        for (Method method : listener.getClass().getMethods()) {
            EventHandler annotation = method.getAnnotation(EventHandler.class);
            if (annotation == null) {
                continue;
            }
            Class<?>[] parameters = method.getParameterTypes();
            if (parameters.length != 1 || !Event.class.isAssignableFrom(parameters[0])) {
                throw new IllegalArgumentException("Method " + method + " must have exactly one Event parameter");
            }
            if (!void.class.equals(method.getReturnType())) {
                throw new IllegalArgumentException("Method " + method + " must return void");
            }
            @SuppressWarnings("unchecked")
            Class<? extends Event> eventType = (Class<? extends Event>) parameters[0];
            method.setAccessible(true);
            EventHandlerWrapper wrapper = new EventHandlerWrapper(listener, method, eventType, annotation.priority(), annotation.ignoreCancelled());
            handlers.computeIfAbsent(eventType, key -> new ArrayList<>()).add(wrapper);
            handlers.get(eventType).sort(Comparator.comparingInt(a -> a.priority.getPriority()));
        }
    }

    public void unregister(Object listener) {
        handlers.values().forEach(list -> list.removeIf(wrapper -> wrapper.listener == listener));
    }

    public void post(Event event) {
        Objects.requireNonNull(event, "event");
        List<EventHandlerWrapper> handlersForEvent = collectHandlers(event.getClass());
        if (handlersForEvent.isEmpty()) {
            return;
        }
        for (EventHandlerWrapper wrapper : handlersForEvent) {
            if (event.isCancelled() && !wrapper.ignoreCancelled) {
                continue;
            }
            try {
                wrapper.method.invoke(wrapper.listener, event);
            } catch (IllegalAccessException | InvocationTargetException e) {
                throw new RuntimeException("Failed to invoke event handler", e);
            }
            if (event instanceof Cancellable && event.isCancelled()) {
                break;
            }
        }
    }

    public void clear() {
        handlers.clear();
    }

    private List<EventHandlerWrapper> collectHandlers(Class<? extends Event> eventClass) {
        if (eventClass == null) {
            return List.of();
        }
        java.util.Set<Class<?>> visited = new java.util.HashSet<>();
        java.util.ArrayDeque<Class<?>> queue = new java.util.ArrayDeque<>();
        java.util.LinkedHashSet<EventHandlerWrapper> collected = new java.util.LinkedHashSet<>();
        if (eventClass != null) {
            queue.add(eventClass);
        }
        while (!queue.isEmpty()) {
            Class<?> current = queue.removeFirst();
            if (current == null || !Event.class.isAssignableFrom(current) || !visited.add(current)) {
                continue;
            }
            @SuppressWarnings("unchecked")
            Class<? extends Event> castCurrent = (Class<? extends Event>) current;
            List<EventHandlerWrapper> wrappers = handlers.get(castCurrent);
            if (wrappers != null) {
                collected.addAll(wrappers);
            }
            Class<?> superclass = current.getSuperclass();
            if (superclass != null) {
                queue.add(superclass);
            }
            for (Class<?> iface : current.getInterfaces()) {
                if (Event.class.isAssignableFrom(iface)) {
                    queue.add(iface);
                }
            }
        }
        if (collected.isEmpty()) {
            return List.of();
        }
        List<EventHandlerWrapper> sorted = new ArrayList<>(collected);
        sorted.sort(Comparator.comparingInt(a -> a.priority.getPriority()));
        return sorted;
    }

    private static class EventHandlerWrapper {
        private final Object listener;
        private final Method method;
        private final Class<? extends Event> eventType;
        private final EventPriority priority;
        private final boolean ignoreCancelled;

        private EventHandlerWrapper(Object listener, Method method, Class<? extends Event> eventType,
                                    EventPriority priority, boolean ignoreCancelled) {
            this.listener = listener;
            this.method = method;
            this.eventType = eventType;
            this.priority = priority;
            this.ignoreCancelled = ignoreCancelled;
        }
    }
}
