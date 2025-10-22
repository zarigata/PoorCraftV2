package com.poorcraft.common.registry;

import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

public class RegistryManager {
    public static final String BLOCKS = "blocks";
    public static final String ITEMS = "items";
    public static final String ENTITIES = "entities";
    public static final String BIOMES = "biomes";

    private final Map<String, Registry<?>> registries = new HashMap<>();
    private boolean frozen;

    public <T> Registry<T> createRegistry(String name, Class<T> type) {
        Objects.requireNonNull(name, "name");
        Objects.requireNonNull(type, "type");
        if (registries.containsKey(name)) {
            throw new IllegalStateException("Registry '" + name + "' already exists");
        }
        if (frozen) {
            throw new IllegalStateException("RegistryManager is frozen");
        }
        Registry<T> registry = new Registry<>(name);
        registries.put(name, registry);
        return registry;
    }

    @SuppressWarnings("unchecked")
    public <T> Registry<T> getRegistry(String name) {
        Objects.requireNonNull(name, "name");
        return (Registry<T>) registries.get(name);
    }

    public Collection<Registry<?>> getAllRegistries() {
        return Collections.unmodifiableCollection(registries.values());
    }

    public void freezeAll() {
        registries.values().forEach(Registry::freeze);
        frozen = true;
    }

    public boolean isFrozen() {
        return frozen;
    }

    public Registry<BlockDefinition> getBlockRegistry() {
        return getRegistry(BLOCKS);
    }

    public Registry<ItemDefinition> getItemRegistry() {
        return getRegistry(ITEMS);
    }

    public Registry<EntityDefinition> getEntityRegistry() {
        return getRegistry(ENTITIES);
    }
}
