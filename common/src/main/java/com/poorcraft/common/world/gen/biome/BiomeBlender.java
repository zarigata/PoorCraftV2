package com.poorcraft.common.world.gen.biome;

import com.poorcraft.common.world.block.BlockType;

/**
 * Handles biome parameter blending using inverse-distance weighting.
 */
public class BiomeBlender {
    
    private final double blendRadius;
    private static final double EPSILON = 0.0001;
    
    /**
     * Creates a new biome blender.
     * 
     * @param blendRadius Radius for biome parameter blending
     */
    public BiomeBlender(double blendRadius) {
        this.blendRadius = blendRadius;
    }
    
    /**
     * Selects the closest biome based on parameter distance.
     * 
     * @param elevation Elevation value [0, 1]
     * @param moisture Moisture value [0, 1]
     * @param temperature Temperature value [0, 1]
     * @return Biome ID
     */
    public int selectBiome(double elevation, double moisture, double temperature) {
        BiomeType closestBiome = BiomeType.PLAINS;
        double minDistance = Double.MAX_VALUE;
        
        for (BiomeType biome : BiomeType.getAllBiomes()) {
            double de = elevation - biome.getElevation();
            double dm = moisture - biome.getMoisture();
            double dt = temperature - biome.getTemperature();
            double distanceSq = de * de + dm * dm + dt * dt;
            
            if (distanceSq < minDistance) {
                minDistance = distanceSq;
                closestBiome = biome;
            }
        }
        
        return closestBiome.getId();
    }
    
    /**
     * Blends biome parameters using inverse-distance weighting.
     * 
     * @param elevation Elevation value [0, 1]
     * @param moisture Moisture value [0, 1]
     * @param temperature Temperature value [0, 1]
     * @return Blended biome parameters
     */
    public BlendedBiomeParams blendParameters(double elevation, double moisture, double temperature) {
        double sumWeight = 0.0;
        double sumBaseHeight = 0.0;
        double sumRoughness = 0.0;
        
        // Track block type votes
        double[] surfaceWeights = new double[BiomeType.getAllBiomes().length];
        double[] fillWeights = new double[BiomeType.getAllBiomes().length];
        
        BiomeType dominantBiome = BiomeType.PLAINS;
        double maxWeight = 0.0;
        
        for (BiomeType biome : BiomeType.getAllBiomes()) {
            double de = elevation - biome.getElevation();
            double dm = moisture - biome.getMoisture();
            double dt = temperature - biome.getTemperature();
            double distanceSq = de * de + dm * dm + dt * dt;
            
            // Inverse distance weighting
            double weight = 1.0 / Math.max(distanceSq, EPSILON);
            
            sumWeight += weight;
            sumBaseHeight += biome.getBaseHeight() * weight;
            sumRoughness += biome.getRoughness() * weight;
            
            surfaceWeights[biome.getId()] += weight;
            fillWeights[biome.getId()] += weight;
            
            if (weight > maxWeight) {
                maxWeight = weight;
                dominantBiome = biome;
            }
        }
        
        // Normalize
        double baseHeight = sumBaseHeight / sumWeight;
        double roughness = sumRoughness / sumWeight;
        
        // Determine dominant block types by weight
        BlockType surfaceBlock = dominantBiome.getSurfaceBlock();
        BlockType fillBlock = dominantBiome.getFillBlock();
        
        double maxSurfaceWeight = 0.0;
        double maxFillWeight = 0.0;
        
        for (BiomeType biome : BiomeType.getAllBiomes()) {
            if (surfaceWeights[biome.getId()] > maxSurfaceWeight) {
                maxSurfaceWeight = surfaceWeights[biome.getId()];
                surfaceBlock = biome.getSurfaceBlock();
            }
            if (fillWeights[biome.getId()] > maxFillWeight) {
                maxFillWeight = fillWeights[biome.getId()];
                fillBlock = biome.getFillBlock();
            }
        }
        
        return new BlendedBiomeParams(baseHeight, roughness, surfaceBlock, fillBlock, dominantBiome);
    }
    
    /**
     * Container for blended biome parameters.
     */
    public static class BlendedBiomeParams {
        public final double baseHeight;
        public final double roughness;
        public final BlockType surfaceBlock;
        public final BlockType fillBlock;
        public final BiomeType dominantBiome;
        
        public BlendedBiomeParams(double baseHeight, double roughness,
                                  BlockType surfaceBlock, BlockType fillBlock,
                                  BiomeType dominantBiome) {
            this.baseHeight = baseHeight;
            this.roughness = roughness;
            this.surfaceBlock = surfaceBlock;
            this.fillBlock = fillBlock;
            this.dominantBiome = dominantBiome;
        }
    }
}
