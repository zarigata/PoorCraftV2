package com.poorcraft.common.registry;

import com.poorcraft.common.entity.EntityType;
import com.poorcraft.common.inventory.ItemType;
import com.poorcraft.common.world.block.BlockType;

public final class Registries {
    private static final RegistryManager INSTANCE = new RegistryManager();

    private Registries() {
    }

    public static RegistryManager getInstance() {
        return INSTANCE;
    }

    public static void init() {
        Registry<BlockDefinition> blockRegistry = INSTANCE.createRegistry(RegistryManager.BLOCKS, BlockDefinition.class);
        Registry<ItemDefinition> itemRegistry = INSTANCE.createRegistry(RegistryManager.ITEMS, ItemDefinition.class);
        Registry<EntityDefinition> entityRegistry = INSTANCE.createRegistry(RegistryManager.ENTITIES, EntityDefinition.class);

        registerVanillaBlocks(blockRegistry);
        registerVanillaItems(itemRegistry, blockRegistry);
        registerVanillaEntities(entityRegistry);
    }

    public static Registry<BlockDefinition> blocks() {
        return INSTANCE.getBlockRegistry();
    }

    public static Registry<ItemDefinition> items() {
        return INSTANCE.getItemRegistry();
    }

    public static Registry<EntityDefinition> entities() {
        return INSTANCE.getEntityRegistry();
    }

    private static void registerVanillaBlocks(Registry<BlockDefinition> registry) {
        for (BlockType blockType : BlockType.values()) {
            BlockDefinition definition = BlockDefinition.fromBlockType(blockType);
            registry.register(definition.getId(), blockType.getId(), definition);
        }
    }

    private static void registerVanillaItems(Registry<ItemDefinition> itemRegistry, Registry<BlockDefinition> blockRegistry) {
        for (ItemType itemType : ItemType.values()) {
            BlockDefinition blockDefinition = null;
            if (itemType.getBlockType() != null) {
                blockDefinition = blockRegistry.get("minecraft:" + itemType.getBlockType().getName());
            }
            ItemDefinition itemDefinition = ItemDefinition.fromItemType(itemType, blockDefinition);
            itemRegistry.register(itemDefinition.getId(), itemType.getId(), itemDefinition);
        }
    }

    private static void registerVanillaEntities(Registry<EntityDefinition> registry) {
        for (EntityType entityType : EntityType.values()) {
            EntityDefinition definition = EntityDefinition.fromEntityType(entityType);
            registry.register(definition.getId(), entityType.getId(), definition);
        }
    }
}
