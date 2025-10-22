package com.poorcraft.common.registry;

import java.util.Collection;
import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.regex.Pattern;

public class Registry<T> {
    private static final Pattern ID_PATTERN = Pattern.compile("[a-z0-9_\\-]+:[a-z0-9_\\-/]+");

    private final String registryName;
    private final Map<String, T> entries = new LinkedHashMap<>();
    private final Map<Integer, T> entriesById = new LinkedHashMap<>();
    private final AtomicInteger nextId = new AtomicInteger();
    private volatile boolean frozen;

    public Registry(String registryName) {
        this.registryName = Objects.requireNonNull(registryName, "registryName");
    }

    public synchronized T register(String id, T value) {
        int numericId = nextId.getAndIncrement();
        return registerInternal(id, numericId, value, false);
    }

    public synchronized T register(String id, int numericId, T value) {
        if (numericId < 0) {
            throw new IllegalArgumentException("Numeric identifier must be non-negative");
        }
        if (nextId.get() <= numericId) {
            nextId.set(numericId + 1);
        }
        return registerInternal(id, numericId, value, true);
    }

    private T registerInternal(String id, int numericId, T value, boolean explicitId) {
        Objects.requireNonNull(id, "id");
        Objects.requireNonNull(value, "value");
        if (frozen) {
            throw new IllegalStateException("Registry '" + registryName + "' is frozen");
        }
        if (!ID_PATTERN.matcher(id).matches()) {
            throw new IllegalArgumentException("Invalid identifier '" + id + "' for registry '" + registryName + "'");
        }
        if (entries.containsKey(id)) {
            throw new IllegalStateException("Duplicate identifier '" + id + "' in registry '" + registryName + "'");
        }
        if (entriesById.containsKey(numericId)) {
            String message = explicitId
                    ? "Duplicate numeric identifier '" + numericId + "' in registry '" + registryName + "'"
                    : "Auto-assigned numeric identifier collision in registry '" + registryName + "'";
            throw new IllegalStateException(message);
        }
        entries.put(id, value);
        entriesById.put(numericId, value);
        if (value instanceof RegistryObject) {
            ((RegistryObject) value).setNumericId(numericId);
        }
        return value;
    }

    public T get(String id) {
        return entries.get(id);
    }

    public T get(int id) {
        return entriesById.get(id);
    }

    public boolean contains(String id) {
        return entries.containsKey(id);
    }

    public Collection<T> getAll() {
        return Collections.unmodifiableCollection(entries.values());
    }

    public T getOrDefault(String id, T defaultValue) {
        return entries.getOrDefault(id, defaultValue);
    }

    public synchronized void freeze() {
        frozen = true;
    }

    public boolean isFrozen() {
        return frozen;
    }

    public String getRegistryName() {
        return registryName;
    }

    public interface RegistryObject {
        void setNumericId(int id);

        int getNumericId();
    }
}
