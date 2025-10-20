package com.poorcraft.common.world.gen.noise;

/**
 * Wrapper for noise generation with Fractal Brownian Motion (FBM).
 */
public class NoiseGenerator {
    
    private final long seed;
    private final int octaves;
    private final double lacunarity;
    private final double gain;
    private final double scale;
    private final OpenSimplex2S noise;
    
    /**
     * Creates a new noise generator.
     * 
     * @param seed Random seed
     * @param octaves Number of octaves for FBM
     * @param lacunarity Frequency multiplier per octave
     * @param gain Amplitude multiplier per octave
     * @param scale Base scale for noise coordinates
     */
    public NoiseGenerator(long seed, int octaves, double lacunarity, double gain, double scale) {
        this.seed = seed;
        this.octaves = octaves;
        this.lacunarity = lacunarity;
        this.gain = gain;
        this.scale = scale;
        this.noise = new OpenSimplex2S(seed);
    }
    
    /**
     * Generates 2D FBM noise.
     * 
     * @param x X coordinate
     * @param z Z coordinate
     * @return Noise value normalized to approximately [-1, 1]
     */
    public double fbm2D(double x, double z) {
        double amplitude = 1.0;
        double frequency = 1.0;
        double sum = 0.0;
        double normalization = 0.0;
        
        for (int i = 0; i < octaves; i++) {
            double sampleX = x * frequency * scale;
            double sampleZ = z * frequency * scale;
            double value = noise.noise2(seed, sampleX, sampleZ);
            
            sum += value * amplitude;
            normalization += amplitude;
            
            amplitude *= gain;
            frequency *= lacunarity;
        }
        
        return sum / normalization;
    }
    
    /**
     * Generates 3D FBM noise using ImproveXZ variant.
     * 
     * @param x X coordinate
     * @param y Y coordinate
     * @param z Z coordinate
     * @return Noise value normalized to approximately [-1, 1]
     */
    public double fbm3D(double x, double y, double z) {
        double amplitude = 1.0;
        double frequency = 1.0;
        double sum = 0.0;
        double normalization = 0.0;
        
        for (int i = 0; i < octaves; i++) {
            double sampleX = x * frequency * scale;
            double sampleY = y * frequency * scale;
            double sampleZ = z * frequency * scale;
            double value = noise.noise3_ImproveXZ(seed, sampleX, sampleY, sampleZ);
            
            sum += value * amplitude;
            normalization += amplitude;
            
            amplitude *= gain;
            frequency *= lacunarity;
        }
        
        return sum / normalization;
    }
    
    /**
     * Gets elevation noise for terrain generation.
     */
    public double getElevation(double x, double z) {
        return fbm2D(x, z);
    }
    
    /**
     * Gets moisture noise for biome selection.
     */
    public double getMoisture(double x, double z) {
        return fbm2D(x + 10000, z + 10000);
    }
    
    /**
     * Gets temperature noise for biome selection.
     */
    public double getTemperature(double x, double z) {
        return fbm2D(x + 20000, z + 20000);
    }
    
    public long getSeed() {
        return seed;
    }
    
    public int getOctaves() {
        return octaves;
    }
    
    public double getLacunarity() {
        return lacunarity;
    }
    
    public double getGain() {
        return gain;
    }
    
    public double getScale() {
        return scale;
    }
}
