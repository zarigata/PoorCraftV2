package com.poorcraft.common.world.gen.biome;

import com.poorcraft.common.Constants;
import com.poorcraft.common.world.block.BlockType;

/**
 * Enum representing the 5 biomes in the game.
 */
public enum BiomeType {
    PLAINS(
        Constants.Biomes.PLAINS_ID,
        "plains",
        0.45, 0.50, 0.60,
        0.03, 0.30,
        BlockType.GRASS, BlockType.DIRT,
        0x7CFC00
    ),
    DESERT(
        Constants.Biomes.DESERT_ID,
        "desert",
        0.55, 0.10, 0.90,
        0.00, 0.35,
        BlockType.SAND, BlockType.SANDSTONE,
        0xF4A460
    ),
    MOUNTAINS(
        Constants.Biomes.MOUNTAINS_ID,
        "mountains",
        0.70, 0.40, 0.40,
        0.20, 0.60,
        BlockType.STONE, BlockType.STONE,
        0x808080
    ),
    SNOW(
        Constants.Biomes.SNOW_ID,
        "snow",
        0.60, 0.30, 0.15,
        0.10, 0.50,
        BlockType.SNOW, BlockType.STONE,
        0xF0F8FF
    ),
    JUNGLE(
        Constants.Biomes.JUNGLE_ID,
        "jungle",
        0.50, 0.80, 0.85,
        0.05, 0.45,
        BlockType.GRASS, BlockType.DIRT,
        0x228B22
    );
    
    private static final BiomeType[] BY_ID = new BiomeType[Constants.Biomes.BIOME_COUNT];
    
    static {
        for (BiomeType type : values()) {
            BY_ID[type.id] = type;
        }
    }
    
    private final int id;
    private final String name;
    private final double elevation;
    private final double moisture;
    private final double temperature;
    private final double baseHeight;
    private final double roughness;
    private final BlockType surfaceBlock;
    private final BlockType fillBlock;
    private final int color;
    
    BiomeType(int id, String name,
              double elevation, double moisture, double temperature,
              double baseHeight, double roughness,
              BlockType surfaceBlock, BlockType fillBlock,
              int color) {
        this.id = id;
        this.name = name;
        this.elevation = elevation;
        this.moisture = moisture;
        this.temperature = temperature;
        this.baseHeight = baseHeight;
        this.roughness = roughness;
        this.surfaceBlock = surfaceBlock;
        this.fillBlock = fillBlock;
        this.color = color;
    }
    
    public int getId() {
        return id;
    }
    
    public String getName() {
        return name;
    }
    
    public double getElevation() {
        return elevation;
    }
    
    public double getMoisture() {
        return moisture;
    }
    
    public double getTemperature() {
        return temperature;
    }
    
    public double getBaseHeight() {
        return baseHeight;
    }
    
    public double getRoughness() {
        return roughness;
    }
    
    public BlockType getSurfaceBlock() {
        return surfaceBlock;
    }
    
    public BlockType getFillBlock() {
        return fillBlock;
    }
    
    public int getColor() {
        return color;
    }
    
    /**
     * Gets a biome type by its ID.
     * 
     * @param id Biome ID
     * @return BiomeType, or PLAINS if invalid
     */
    public static BiomeType getById(int id) {
        if (id < 0 || id >= BY_ID.length || BY_ID[id] == null) {
            return PLAINS;
        }
        return BY_ID[id];
    }
    
    /**
     * Gets all biome types.
     */
    public static BiomeType[] getAllBiomes() {
        return values();
    }
}
