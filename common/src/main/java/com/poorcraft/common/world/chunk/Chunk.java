package com.poorcraft.common.world.chunk;

import com.poorcraft.common.Constants;
import com.poorcraft.common.world.block.BlockType;

/**
 * Represents a 16x16x256 chunk column.
 */
public class Chunk {
    
    private final int chunkX;
    private final int chunkZ;
    private final ChunkSection[] sections;
    private final byte[] biomes; // 16x16 biome map
    private final int[] heightMap; // 16x16 height map
    private boolean generated;
    private boolean modified;
    
    /**
     * Creates a new chunk.
     * 
     * @param chunkX Chunk X coordinate
     * @param chunkZ Chunk Z coordinate
     */
    public Chunk(int chunkX, int chunkZ) {
        this.chunkX = chunkX;
        this.chunkZ = chunkZ;
        this.sections = new ChunkSection[Constants.World.SECTIONS_PER_CHUNK];
        this.biomes = new byte[16 * 16];
        this.heightMap = new int[16 * 16];
        this.generated = false;
        this.modified = false;
    }
    
    /**
     * Sets a block in this chunk.
     * 
     * @param x Local X coordinate (0-15)
     * @param y World Y coordinate (0-255)
     * @param z Local Z coordinate (0-15)
     * @param blockId Block ID
     */
    public void setBlock(int x, int y, int z, int blockId) {
        if (x < 0 || x >= 16 || y < 0 || y >= Constants.World.CHUNK_SIZE_Y || z < 0 || z >= 16) {
            return;
        }
        
        int sectionIndex = y / 16;
        int localY = y % 16;
        
        if (sections[sectionIndex] == null) {
            sections[sectionIndex] = new ChunkSection(sectionIndex);
        }
        
        sections[sectionIndex].setBlock(x, localY, z, blockId);
        modified = true;
        
        // Update height map if this is a solid block
        BlockType type = BlockType.getById(blockId);
        if (type.isSolid()) {
            int index = z * 16 + x;
            if (y > heightMap[index]) {
                heightMap[index] = y;
            }
        } else if (blockId == 0) {
            // If removing a block, might need to recalculate height
            int index = z * 16 + x;
            if (y == heightMap[index]) {
                updateHeightMapAt(x, z);
            }
        }
    }
    
    /**
     * Gets a block from this chunk.
     * 
     * @param x Local X coordinate (0-15)
     * @param y World Y coordinate (0-255)
     * @param z Local Z coordinate (0-15)
     * @return Block ID
     */
    public int getBlock(int x, int y, int z) {
        if (x < 0 || x >= 16 || y < 0 || y >= Constants.World.CHUNK_SIZE_Y || z < 0 || z >= 16) {
            return 0;
        }
        
        int sectionIndex = y / 16;
        if (sections[sectionIndex] == null) {
            return 0; // Empty section = all air
        }
        
        int localY = y % 16;
        return sections[sectionIndex].getBlock(x, localY, z);
    }
    
    /**
     * Gets the block type at the specified position.
     * 
     * @param x Local X coordinate (0-15)
     * @param y World Y coordinate (0-255)
     * @param z Local Z coordinate (0-15)
     * @return BlockType
     */
    public BlockType getBlockType(int x, int y, int z) {
        return BlockType.getById(getBlock(x, y, z));
    }
    
    /**
     * Sets the biome for a column.
     * 
     * @param x Local X coordinate (0-15)
     * @param z Local Z coordinate (0-15)
     * @param biomeId Biome ID
     */
    public void setBiome(int x, int z, byte biomeId) {
        if (x >= 0 && x < 16 && z >= 0 && z < 16) {
            biomes[z * 16 + x] = biomeId;
        }
    }
    
    /**
     * Gets the biome for a column.
     * 
     * @param x Local X coordinate (0-15)
     * @param z Local Z coordinate (0-15)
     * @return Biome ID
     */
    public byte getBiome(int x, int z) {
        if (x >= 0 && x < 16 && z >= 0 && z < 16) {
            return biomes[z * 16 + x];
        }
        return 0;
    }
    
    /**
     * Gets the height of the highest solid block at the specified column.
     * 
     * @param x Local X coordinate (0-15)
     * @param z Local Z coordinate (0-15)
     * @return Height (Y coordinate)
     */
    public int getHeight(int x, int z) {
        if (x >= 0 && x < 16 && z >= 0 && z < 16) {
            return heightMap[z * 16 + x];
        }
        return 0;
    }
    
    /**
     * Updates the height map for the entire chunk.
     */
    public void updateHeightMap() {
        for (int x = 0; x < 16; x++) {
            for (int z = 0; z < 16; z++) {
                updateHeightMapAt(x, z);
            }
        }
    }
    
    /**
     * Updates the height map for a specific column.
     */
    private void updateHeightMapAt(int x, int z) {
        int index = z * 16 + x;
        for (int y = Constants.World.CHUNK_SIZE_Y - 1; y >= 0; y--) {
            BlockType type = getBlockType(x, y, z);
            if (type.isSolid()) {
                heightMap[index] = y;
                return;
            }
        }
        heightMap[index] = 0;
    }
    
    /**
     * Gets a chunk section by index.
     * 
     * @param index Section index (0-15)
     * @return ChunkSection, or null if empty
     */
    public ChunkSection getSection(int index) {
        if (index >= 0 && index < sections.length) {
            return sections[index];
        }
        return null;
    }
    
    /**
     * Gets all sections.
     */
    public ChunkSection[] getSections() {
        return sections;
    }
    
    public int getChunkX() {
        return chunkX;
    }
    
    public int getChunkZ() {
        return chunkZ;
    }
    
    public boolean isGenerated() {
        return generated;
    }
    
    public void setGenerated(boolean generated) {
        this.generated = generated;
    }
    
    public boolean isModified() {
        return modified;
    }
    
    public void setModified(boolean modified) {
        this.modified = modified;
    }
    
    /**
     * Gets memory usage in bytes.
     */
    public long getMemoryUsage() {
        long total = 0;
        for (ChunkSection section : sections) {
            if (section != null) {
                total += section.getMemoryUsage();
            }
        }
        total += biomes.length + heightMap.length * 4L;
        return total;
    }
}
