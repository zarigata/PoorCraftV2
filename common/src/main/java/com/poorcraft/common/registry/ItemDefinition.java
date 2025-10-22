package com.poorcraft.common.registry;

import com.poorcraft.common.inventory.ItemType;
import com.poorcraft.common.registry.Registry.RegistryObject;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

public class ItemDefinition implements RegistryObject {
    private final String id;
    private final String name;
    private final boolean stackable;
    private final int maxStackSize;
    private final boolean placeable;
    private final BlockDefinition blockDefinition;
    private final int durability;
    private final Map<String, Object> properties;
    private int numericId = -1;

    private ItemDefinition(Builder builder) {
        this.id = Objects.requireNonNull(builder.id, "id");
        this.name = Objects.requireNonNull(builder.name, "name");
        this.stackable = builder.stackable;
        this.maxStackSize = builder.maxStackSize;
        this.placeable = builder.placeable;
        this.blockDefinition = builder.blockDefinition;
        this.durability = builder.durability;
        this.properties = Collections.unmodifiableMap(new HashMap<>(builder.properties));
    }

    public String getId() {
        return id;
    }

    @Override
    public void setNumericId(int id) {
        this.numericId = id;
    }

    @Override
    public int getNumericId() {
        return numericId;
    }

    public String getName() {
        return name;
    }

    public boolean isStackable() {
        return stackable;
    }

    public int getMaxStackSize() {
        return maxStackSize;
    }

    public boolean isPlaceable() {
        return placeable;
    }

    public BlockDefinition getBlockDefinition() {
        return blockDefinition;
    }

    public int getDurability() {
        return durability;
    }

    public Map<String, Object> getProperties() {
        return properties;
    }

    public static ItemDefinition fromItemType(ItemType itemType, BlockDefinition blockDefinition) {
        Builder builder = new Builder("minecraft:" + itemType.name().toLowerCase())
                .name(itemType.getName())
                .stackable(itemType.isStackable())
                .maxStackSize(itemType.getMaxStackSize())
                .placeable(itemType.isPlaceable())
                .durability(-1)
                .block(blockDefinition);
        return builder.build();
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof ItemDefinition)) return false;
        ItemDefinition that = (ItemDefinition) o;
        return id.equals(that.id);
    }

    @Override
    public int hashCode() {
        return id.hashCode();
    }

    public static Builder builder(String id) {
        return new Builder(id);
    }

    public static class Builder {
        private final String id;
        private String name;
        private boolean stackable = true;
        private int maxStackSize = 64;
        private boolean placeable;
        private BlockDefinition blockDefinition;
        private int durability = -1;
        private final Map<String, Object> properties = new HashMap<>();

        private Builder(String id) {
            this.id = id;
        }

        public Builder name(String name) {
            this.name = name;
            return this;
        }

        public Builder stackable(boolean stackable) {
            this.stackable = stackable;
            return this;
        }

        public Builder maxStackSize(int maxStackSize) {
            this.maxStackSize = maxStackSize;
            return this;
        }

        public Builder placeable(boolean placeable) {
            this.placeable = placeable;
            return this;
        }

        public Builder block(BlockDefinition blockDefinition) {
            this.blockDefinition = blockDefinition;
            return this;
        }

        public Builder durability(int durability) {
            this.durability = durability;
            return this;
        }

        public Builder property(String key, Object value) {
            this.properties.put(key, value);
            return this;
        }

        public ItemDefinition build() {
            return new ItemDefinition(this);
        }
    }
}
