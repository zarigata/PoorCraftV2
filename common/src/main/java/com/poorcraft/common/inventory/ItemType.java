package com.poorcraft.common.inventory;

import java.util.HashMap;
import java.util.Map;

import com.poorcraft.common.world.block.BlockType;

public enum ItemType {

    AIR(0, "Air", false, 0, false, null),
    STONE(1, "Stone", true, 64, true, BlockType.STONE),
    DIRT(2, "Dirt", true, 64, true, BlockType.DIRT),
    GRASS(3, "Grass", true, 64, true, BlockType.GRASS),
    SAND(4, "Sand", true, 64, true, BlockType.SAND),
    WOOD(8, "Wood", true, 64, true, BlockType.WOOD),
    LEAVES(9, "Leaves", true, 64, true, BlockType.LEAVES);

    private static final Map<Integer, ItemType> BY_ID = new HashMap<>();

    static {
        for (ItemType type : values()) {
            BY_ID.put(type.id, type);
        }
    }

    private final int id;
    private final String name;
    private final boolean stackable;
    private final int maxStackSize;
    private final boolean placeable;
    private final BlockType blockType;

    ItemType(int id, String name, boolean stackable, int maxStackSize, boolean placeable, BlockType blockType) {
        this.id = id;
        this.name = name;
        this.stackable = stackable;
        this.maxStackSize = maxStackSize;
        this.placeable = placeable;
        this.blockType = blockType;
    }

    public int getId() {
        return id;
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

    public BlockType getBlockType() {
        return blockType;
    }

    public static ItemType getById(int id) {
        return BY_ID.getOrDefault(id, AIR);
    }

    public static ItemType fromBlockType(BlockType blockType) {
        for (ItemType type : values()) {
            if (type.blockType == blockType) {
                return type;
            }
        }
        return AIR;
    }
}
