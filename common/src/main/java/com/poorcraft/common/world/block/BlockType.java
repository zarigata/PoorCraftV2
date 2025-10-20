package com.poorcraft.common.world.block;

import com.poorcraft.common.Constants;

/**
 * Enum representing all block types in the game.
 */
public enum BlockType {
    AIR(Constants.Blocks.AIR_ID, "air", true, false, false, ""),
    STONE(Constants.Blocks.STONE_ID, "stone", false, true, false, "stone"),
    DIRT(Constants.Blocks.DIRT_ID, "dirt", false, true, false, "dirt"),
    GRASS(Constants.Blocks.GRASS_ID, "grass", false, true, false, "grass_top", "grass_side", "dirt"),
    SAND(Constants.Blocks.SAND_ID, "sand", false, true, false, "sand"),
    SANDSTONE(Constants.Blocks.SANDSTONE_ID, "sandstone", false, true, false, "sandstone"),
    SNOW(Constants.Blocks.SNOW_ID, "snow", false, true, false, "snow"),
    ICE(Constants.Blocks.ICE_ID, "ice", true, true, false, "ice"),
    WOOD(Constants.Blocks.WOOD_ID, "wood", false, true, false, "wood_top", "wood_side", "wood_top"),
    LEAVES(Constants.Blocks.LEAVES_ID, "leaves", true, true, false, "leaves"),
    WATER(Constants.Blocks.WATER_ID, "water", true, false, true, "water");
    
    private static final BlockType[] BY_ID = new BlockType[256];
    
    static {
        for (BlockType type : values()) {
            BY_ID[type.id] = type;
        }
    }
    
    private final int id;
    private final String name;
    private final boolean transparent;
    private final boolean solid;
    private final boolean liquid;
    private final String topTexture;
    private final String sideTexture;
    private final String bottomTexture;
    
    BlockType(int id, String name, boolean transparent, boolean solid, boolean liquid, String texture) {
        this(id, name, transparent, solid, liquid, texture, texture, texture);
    }
    
    BlockType(int id, String name, boolean transparent, boolean solid, boolean liquid,
              String topTexture, String sideTexture, String bottomTexture) {
        this.id = id;
        this.name = name;
        this.transparent = transparent;
        this.solid = solid;
        this.liquid = liquid;
        this.topTexture = topTexture;
        this.sideTexture = sideTexture;
        this.bottomTexture = bottomTexture;
    }
    
    public int getId() {
        return id;
    }
    
    public String getName() {
        return name;
    }
    
    public boolean isTransparent() {
        return transparent;
    }
    
    public boolean isSolid() {
        return solid;
    }
    
    public boolean isLiquid() {
        return liquid;
    }
    
    public boolean isAir() {
        return this == AIR;
    }
    
    /**
     * Gets the texture name for a specific face.
     * 
     * @param face 0=bottom, 1=top, 2-5=sides
     * @return Texture name
     */
    public String getTextureName(int face) {
        if (face == 0) return bottomTexture;
        if (face == 1) return topTexture;
        return sideTexture;
    }
    
    /**
     * Gets a block type by its ID.
     * 
     * @param id Block ID
     * @return BlockType, or AIR if invalid
     */
    public static BlockType getById(int id) {
        if (id < 0 || id >= BY_ID.length || BY_ID[id] == null) {
            return AIR;
        }
        return BY_ID[id];
    }
}
