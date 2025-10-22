package com.poorcraft.common.mod.script;

import com.poorcraft.api.ModAPI;
import com.poorcraft.common.event.EventBus;
import com.poorcraft.common.registry.BlockDefinition;
import com.poorcraft.common.registry.EntityDefinition;
import com.poorcraft.common.registry.ItemDefinition;
import com.poorcraft.common.registry.Registry;

import java.util.Objects;

public class ScriptGameAPI {
    private final String modId;

    public ScriptGameAPI(String modId) {
        this.modId = Objects.requireNonNull(modId, "modId");
    }

    public EventBus getEventBus() {
        return ModAPI.getEventBus();
    }

    public Registry<BlockDefinition> getBlockRegistry() {
        return ModAPI.getBlockRegistry();
    }

    public Registry<ItemDefinition> getItemRegistry() {
        return ModAPI.getItemRegistry();
    }

    public Registry<EntityDefinition> getEntityRegistry() {
        return ModAPI.getEntityRegistry();
    }

    public org.slf4j.Logger getLogger() {
        return ModAPI.getLogger(modId);
    }

    public void registerBlock(BlockDefinition definition) {
        ModAPI.registerBlock(modId, definition);
    }

    public void registerItem(ItemDefinition definition) {
        ModAPI.registerItem(modId, definition);
    }

    public void registerEntity(EntityDefinition definition) {
        ModAPI.registerEntity(modId, definition);
    }
}
