package com.poorcraft.common.world.gen;

import com.poorcraft.common.Constants;
import com.poorcraft.common.config.Configuration;
import com.poorcraft.common.world.block.BlockType;
import com.poorcraft.common.world.chunk.Chunk;
import com.poorcraft.common.world.gen.biome.BiomeBlender;
import com.poorcraft.common.world.gen.biome.BiomeType;
import com.poorcraft.common.world.gen.noise.NoiseGenerator;

/**
 * Main terrain generation class.
 */
public class TerrainGenerator {
    
    private final long seed;
    private final Configuration config;
    private final NoiseGenerator elevationNoise;
    private final NoiseGenerator moistureNoise;
    private final NoiseGenerator temperatureNoise;
    private final NoiseGenerator detailNoise;
    private final BiomeBlender biomeBlender;
    
    private final int baseHeight;
    private final int heightVariation;
    private final double treeChance;
    
    /**
     * Creates a new terrain generator.
     * 
     * @param seed World seed
     * @param config Configuration
     */
    public TerrainGenerator(long seed, Configuration config) {
        this.seed = seed;
        this.config = config;
        
        int octaves = config.getInt("world.noiseOctaves", 6);
        double lacunarity = config.getDouble("world.noiseLacunarity", 2.0);
        double gain = config.getDouble("world.noiseGain", 0.5);
        double scale = config.getDouble("world.noiseScale", 0.003);
        double biomeScale = config.getDouble("world.biomeScale", 0.001);
        
        this.elevationNoise = new NoiseGenerator(seed, octaves, lacunarity, gain, scale);
        this.moistureNoise = new NoiseGenerator(seed + 1000, 4, lacunarity, gain, biomeScale);
        this.temperatureNoise = new NoiseGenerator(seed + 2000, 4, lacunarity, gain, biomeScale);
        this.detailNoise = new NoiseGenerator(seed + 3000, 3, lacunarity, gain, scale * 2);
        
        double blendRadius = config.getDouble("world.biomeBlendRadius", 16.0);
        this.biomeBlender = new BiomeBlender(blendRadius);
        
        this.baseHeight = config.getInt("world.baseHeight", 64);
        this.heightVariation = config.getInt("world.heightVariation", 32);
        this.treeChance = config.getDouble("world.treeChance", 0.02);
    }
    
    /**
     * Generates terrain for a chunk.
     * 
     * @param chunk Chunk to generate
     */
    public void generateChunk(Chunk chunk) {
        int chunkX = chunk.getChunkX();
        int chunkZ = chunk.getChunkZ();
        
        // Generate base terrain
        for (int x = 0; x < 16; x++) {
            for (int z = 0; z < 16; z++) {
                int worldX = chunkX * 16 + x;
                int worldZ = chunkZ * 16 + z;
                
                // Get noise values
                double elevation = elevationNoise.getElevation(worldX, worldZ);
                double moisture = moistureNoise.getMoisture(worldX, worldZ);
                double temperature = temperatureNoise.getTemperature(worldX, worldZ);
                
                // Normalize to [0, 1]
                elevation = (elevation + 1.0) * 0.5;
                moisture = (moisture + 1.0) * 0.5;
                temperature = (temperature + 1.0) * 0.5;
                
                // Adjust temperature by elevation (higher = colder)
                temperature = Math.max(0.0, temperature - elevation * 0.3);
                
                // Blend biome parameters
                BiomeBlender.BlendedBiomeParams params = biomeBlender.blendParameters(elevation, moisture, temperature);
                
                // Calculate final height
                double detail = detailNoise.fbm2D(worldX, worldZ);
                double finalHeight = baseHeight + 
                                   params.baseHeight * heightVariation + 
                                   params.roughness * detail * heightVariation;
                
                int height = (int) Math.round(finalHeight);
                height = Math.max(1, Math.min(Constants.World.CHUNK_SIZE_Y - 1, height));
                
                // Set biome
                chunk.setBiome(x, z, (byte) params.dominantBiome.getId());
                
                // Place bedrock
                chunk.setBlock(x, 0, z, BlockType.STONE.getId());
                
                // Fill terrain
                for (int y = 1; y <= height; y++) {
                    if (y == height) {
                        // Surface block
                        chunk.setBlock(x, y, z, params.surfaceBlock.getId());
                    } else if (y >= height - 3) {
                        // Sub-surface (dirt/sandstone/etc)
                        chunk.setBlock(x, y, z, params.fillBlock.getId());
                    } else {
                        // Deep stone
                        chunk.setBlock(x, y, z, BlockType.STONE.getId());
                    }
                }
                
                // Fill water below sea level
                if (height < Constants.World.SEA_LEVEL) {
                    for (int y = height + 1; y <= Constants.World.SEA_LEVEL; y++) {
                        chunk.setBlock(x, y, z, BlockType.WATER.getId());
                    }
                }
            }
        }
        
        // Update height map
        chunk.updateHeightMap();
        
        // Generate features (trees, etc.)
        generateFeatures(chunk);
        
        chunk.setGenerated(true);
    }
    
    /**
     * Generates features like trees.
     * 
     * @param chunk Chunk to add features to
     */
    private void generateFeatures(Chunk chunk) {
        int chunkX = chunk.getChunkX();
        int chunkZ = chunk.getChunkZ();
        
        // Use a different seed offset for feature placement
        long featureSeed = seed + chunkX * 341873128712L + chunkZ * 132897987541L;
        java.util.Random random = new java.util.Random(featureSeed);
        
        for (int x = 2; x < 14; x++) { // Avoid chunk edges
            for (int z = 2; z < 14; z++) {
                if (random.nextDouble() < treeChance) {
                    int height = chunk.getHeight(x, z);
                    BiomeType biome = BiomeType.getById(chunk.getBiome(x, z));
                    
                    // Only place trees in appropriate biomes and above water
                    if (height > Constants.World.SEA_LEVEL && 
                        (biome == BiomeType.PLAINS || biome == BiomeType.JUNGLE)) {
                        placeTree(chunk, x, height + 1, z, biome);
                    }
                }
            }
        }
    }
    
    /**
     * Places a simple tree.
     * 
     * @param chunk Chunk to place tree in
     * @param x Local X coordinate
     * @param y Base Y coordinate
     * @param z Local Z coordinate
     * @param biome Biome type
     */
    private void placeTree(Chunk chunk, int x, int y, int z, BiomeType biome) {
        int trunkHeight = biome == BiomeType.JUNGLE ? 6 : 4;
        
        // Place trunk
        for (int dy = 0; dy < trunkHeight; dy++) {
            if (y + dy < Constants.World.CHUNK_SIZE_Y) {
                chunk.setBlock(x, y + dy, z, BlockType.WOOD.getId());
            }
        }
        
        // Place leaves
        int leafY = y + trunkHeight;
        int leafRadius = 2;
        
        for (int dx = -leafRadius; dx <= leafRadius; dx++) {
            for (int dz = -leafRadius; dz <= leafRadius; dz++) {
                for (int dy = -1; dy <= 1; dy++) {
                    int lx = x + dx;
                    int ly = leafY + dy;
                    int lz = z + dz;
                    
                    // Check bounds
                    if (lx >= 0 && lx < 16 && lz >= 0 && lz < 16 && 
                        ly >= 0 && ly < Constants.World.CHUNK_SIZE_Y) {
                        
                        // Skip center column (trunk) and corners
                        if ((dx == 0 && dz == 0) || (Math.abs(dx) == leafRadius && Math.abs(dz) == leafRadius)) {
                            continue;
                        }
                        
                        // Only place leaves in air
                        if (chunk.getBlock(lx, ly, lz) == 0) {
                            chunk.setBlock(lx, ly, lz, BlockType.LEAVES.getId());
                        }
                    }
                }
            }
        }
    }
}
