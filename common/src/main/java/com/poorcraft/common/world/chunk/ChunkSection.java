package com.poorcraft.common.world.chunk;

import com.poorcraft.common.Constants;

/**
 * Represents a 16x16x16 chunk section.
 */
public class ChunkSection {
    
    private final PalettedContainer blocks;
    private final int sectionY;
    private boolean isEmpty;
    
    /**
     * Creates a new chunk section.
     * 
     * @param sectionY Y index of this section (0-15)
     */
    public ChunkSection(int sectionY) {
        this.sectionY = sectionY;
        this.blocks = new PalettedContainer(Constants.World.CHUNK_SECTION_SIZE * 
                                           Constants.World.CHUNK_SECTION_SIZE * 
                                           Constants.World.CHUNK_SECTION_SIZE);
        this.isEmpty = true;
    }
    
    /**
     * Sets a block in this section.
     * 
     * @param x Local X coordinate (0-15)
     * @param y Local Y coordinate (0-15)
     * @param z Local Z coordinate (0-15)
     * @param blockId Block ID
     */
    public void setBlock(int x, int y, int z, int blockId) {
        if (x < 0 || x >= 16 || y < 0 || y >= 16 || z < 0 || z >= 16) {
            return;
        }
        
        int index = y * 256 + z * 16 + x; // YZX order
        blocks.set(index, blockId);
        
        if (blockId != 0) {
            isEmpty = false;
        } else {
            // Check if section is now empty
            isEmpty = blocks.isEmpty();
        }
    }
    
    /**
     * Gets a block from this section.
     * 
     * @param x Local X coordinate (0-15)
     * @param y Local Y coordinate (0-15)
     * @param z Local Z coordinate (0-15)
     * @return Block ID
     */
    public int getBlock(int x, int y, int z) {
        if (x < 0 || x >= 16 || y < 0 || y >= 16 || z < 0 || z >= 16) {
            return 0;
        }
        
        int index = y * 256 + z * 16 + x; // YZX order
        return blocks.get(index);
    }
    
    /**
     * Checks if this section is empty (all air).
     */
    public boolean isEmpty() {
        return isEmpty;
    }
    
    /**
     * Gets the Y index of this section.
     */
    public int getSectionY() {
        return sectionY;
    }
    
    /**
     * Gets the paletted container for direct access.
     */
    public PalettedContainer getBlocks() {
        return blocks;
    }
    
    /**
     * Gets memory usage in bytes.
     */
    public long getMemoryUsage() {
        return blocks.getMemoryUsage();
    }
}
