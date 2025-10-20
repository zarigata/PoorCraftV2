package com.poorcraft.common.util;

/**
 * Immutable chunk position representation.
 * Used as a key in chunk maps.
 */
public record ChunkPos(int x, int z) {
    
    /**
     * Converts world coordinates to chunk coordinates.
     * 
     * @param worldX World X coordinate
     * @param worldZ World Z coordinate
     * @return ChunkPos for the given world coordinates
     */
    public static ChunkPos fromWorldPos(int worldX, int worldZ) {
        return new ChunkPos(worldX >> 4, worldZ >> 4);
    }
    
    /**
     * Converts world coordinates to chunk coordinates.
     * 
     * @param worldX World X coordinate
     * @param worldZ World Z coordinate
     * @return ChunkPos for the given world coordinates
     */
    public static ChunkPos fromWorldPos(double worldX, double worldZ) {
        return new ChunkPos((int) Math.floor(worldX) >> 4, (int) Math.floor(worldZ) >> 4);
    }
    
    @Override
    public String toString() {
        return "ChunkPos[" + x + ", " + z + "]";
    }
}
