package com.poorcraft.api;

import com.poorcraft.common.event.EventBus;
import com.poorcraft.common.registry.BlockDefinition;
import com.poorcraft.common.registry.EntityDefinition;
import com.poorcraft.common.registry.ItemDefinition;
import com.poorcraft.common.registry.Registries;
import com.poorcraft.common.registry.Registry;
import com.poorcraft.common.registry.RegistryManager;
import com.poorcraft.common.util.Logger;
import org.slf4j.LoggerFactory;

import java.util.Objects;

public final class ModAPI {
    private static EventBus eventBus;
    private static RegistryManager registryManager;

    private ModAPI() {
    }

    public static void initialize(EventBus bus, RegistryManager manager) {
        eventBus = Objects.requireNonNull(bus, "bus");
        registryManager = Objects.requireNonNull(manager, "manager");
    }

    public static EventBus getEventBus() {
        requireInitialized();
        return eventBus;
    }

    public static Registry<BlockDefinition> getBlockRegistry() {
        requireInitialized();
        return registryManager.getBlockRegistry();
    }

    public static Registry<ItemDefinition> getItemRegistry() {
        requireInitialized();
        return registryManager.getItemRegistry();
    }

    public static Registry<EntityDefinition> getEntityRegistry() {
        requireInitialized();
        return registryManager.getEntityRegistry();
    }

    public static org.slf4j.Logger getLogger(String modId) {
        Objects.requireNonNull(modId, "modId");
        return LoggerFactory.getLogger("mod." + modId);
    }

    public static void registerBlock(String modId, BlockDefinition block) {
        requireInitialized();
        Objects.requireNonNull(modId, "modId");
        Objects.requireNonNull(block, "block");
        getBlockRegistry().register(namespacedId(modId, block.getId()), block);
    }

    public static void registerItem(String modId, ItemDefinition item) {
        requireInitialized();
        Objects.requireNonNull(modId, "modId");
        Objects.requireNonNull(item, "item");
        getItemRegistry().register(namespacedId(modId, item.getId()), item);
    }

    public static void registerEntity(String modId, EntityDefinition entity) {
        requireInitialized();
        Objects.requireNonNull(modId, "modId");
        Objects.requireNonNull(entity, "entity");
        getEntityRegistry().register(namespacedId(modId, entity.getId()), entity);
    }

    private static void requireInitialized() {
        if (eventBus == null || registryManager == null) {
            throw new IllegalStateException("ModAPI has not been initialized");
        }
    }

    private static String namespacedId(String namespace, String id) {
        if (id.contains(":")) {
            return id;
        }
        return namespace + ":" + id;
    }
}
