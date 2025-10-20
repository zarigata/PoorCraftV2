package com.poorcraft.common.world.gen.biome;

import com.poorcraft.common.world.block.BlockType;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

class BiomeBlenderTest {

    @Test
    void blendRadiusLimitsAndExpandsContributions() {
        double elevation = 0.62;
        double moisture = 0.41;
        double temperature = 0.53;

        BiomeBlender tiny = new BiomeBlender(1e-4);
        BiomeBlender wide = new BiomeBlender(10.0);

        int nearestId = tiny.selectBiome(elevation, moisture, temperature);
        BiomeType nearest = BiomeType.getById(nearestId);

        BiomeBlender.BlendedBiomeParams tinyParams = tiny.blendParameters(elevation, moisture, temperature);

        assertEquals(nearest, tinyParams.dominantBiome);
        assertEquals(nearest.getSurfaceBlock(), tinyParams.surfaceBlock);
        assertEquals(nearest.getFillBlock(), tinyParams.fillBlock);
        assertEquals(nearest.getBaseHeight(), tinyParams.baseHeight, 1e-9);
        assertEquals(nearest.getRoughness(), tinyParams.roughness, 1e-9);

        BiomeBlender.BlendedBiomeParams wideParams = wide.blendParameters(elevation, moisture, temperature);

        double radius = 10.0;
        double radiusSq = radius * radius;
        double expectedWeightSum = 0.0;
        double expectedBaseHeight = 0.0;
        double expectedRoughness = 0.0;
        double[] surfaceWeights = new double[BiomeType.getAllBiomes().length];
        double[] fillWeights = new double[BiomeType.getAllBiomes().length];
        double maxWeight = 0.0;
        BiomeType expectedDominant = nearest;

        for (BiomeType biome : BiomeType.getAllBiomes()) {
            double de = elevation - biome.getElevation();
            double dm = moisture - biome.getMoisture();
            double dt = temperature - biome.getTemperature();
            double distanceSq = de * de + dm * dm + dt * dt;
            double normalized = distanceSq / radiusSq;
            if (normalized > 1.0) {
                continue;
            }
            double weight = 1.0 / Math.max(normalized, 0.0001);
            expectedWeightSum += weight;
            expectedBaseHeight += biome.getBaseHeight() * weight;
            expectedRoughness += biome.getRoughness() * weight;
            surfaceWeights[biome.getId()] += weight;
            fillWeights[biome.getId()] += weight;
            if (weight > maxWeight) {
                maxWeight = weight;
                expectedDominant = biome;
            }
        }

        assertTrue(expectedWeightSum > 0.0);

        expectedBaseHeight /= expectedWeightSum;
        expectedRoughness /= expectedWeightSum;

        BlockType expectedSurface = expectedDominant.getSurfaceBlock();
        BlockType expectedFill = expectedDominant.getFillBlock();
        double maxSurface = 0.0;
        double maxFill = 0.0;
        for (BiomeType biome : BiomeType.getAllBiomes()) {
            double surfaceWeight = surfaceWeights[biome.getId()];
            if (surfaceWeight > maxSurface) {
                maxSurface = surfaceWeight;
                expectedSurface = biome.getSurfaceBlock();
            }
            double fillWeight = fillWeights[biome.getId()];
            if (fillWeight > maxFill) {
                maxFill = fillWeight;
                expectedFill = biome.getFillBlock();
            }
        }

        assertEquals(expectedDominant, wideParams.dominantBiome);
        assertEquals(expectedSurface, wideParams.surfaceBlock);
        assertEquals(expectedFill, wideParams.fillBlock);
        assertEquals(expectedBaseHeight, wideParams.baseHeight, 1e-9);
        assertEquals(expectedRoughness, wideParams.roughness, 1e-9);
    }
}
