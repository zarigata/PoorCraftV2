package com.poorcraft.common.world.block;

/**
 * Represents a block instance with type and optional metadata.
 */
public record Block(BlockType type, byte metadata) {
    
    public static final Block AIR = new Block(BlockType.AIR, (byte) 0);
    
    /**
     * Creates a block with no metadata.
     */
    public Block(BlockType type) {
        this(type, (byte) 0);
    }
    
    /**
     * Gets the block ID.
     */
    public int getId() {
        return type.getId();
    }
    
    /**
     * Checks if this block is air.
     */
    public boolean isAir() {
        return type.isAir();
    }
    
    /**
     * Checks if this block is transparent.
     */
    public boolean isTransparent() {
        return type.isTransparent();
    }
    
    /**
     * Checks if this block is solid.
     */
    public boolean isSolid() {
        return type.isSolid();
    }
    
    /**
     * Checks if this block is liquid.
     */
    public boolean isLiquid() {
        return type.isLiquid();
    }
}
