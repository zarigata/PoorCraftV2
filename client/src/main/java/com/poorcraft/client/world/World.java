package com.poorcraft.client.world;

import com.poorcraft.client.render.camera.Camera;
import com.poorcraft.client.render.shader.ShaderProgram;
import com.poorcraft.client.render.texture.TextureAtlas;
import com.poorcraft.client.resource.ShaderManager;
import com.poorcraft.client.resource.TextureManager;
import com.poorcraft.common.config.Configuration;
import com.poorcraft.common.util.ChunkPos;
import com.poorcraft.common.world.gen.TerrainGenerator;
import com.poorcraft.core.Renderable;
import com.poorcraft.core.Updatable;
import org.slf4j.Logger;

import java.awt.HeadlessException;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;
import java.util.HashSet;
import java.util.Set;

/**
 * Main World class that coordinates all world systems.
 */
public class World implements Updatable, Renderable {
    
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(World.class);
    
    private final long seed;
    private final Configuration config;
    private final TerrainGenerator terrainGenerator;
    private final ChunkLoader chunkLoader;
    private final ChunkRenderer chunkRenderer;
    private final Camera camera;
    private final TextureAtlas blockAtlas;
    private final ShaderProgram worldShader;
    private final Set<String> missingTextureWarnings = new HashSet<>();
    
    /**
     * Creates a new world.
     * 
     * @param seed World seed
     * @param config Configuration
     * @param camera Camera for rendering
     * @param shaderManager Shader manager
     * @param textureManager Texture manager
     */
    public World(long seed, Configuration config, Camera camera, 
                 ShaderManager shaderManager, TextureManager textureManager) {
        this.seed = seed;
        this.config = config;
        this.camera = camera;
        
        LOGGER.info("Initializing world with seed: {}", seed);
        
        // Initialize terrain generator
        this.terrainGenerator = new TerrainGenerator(seed, config);
        
        // Load block textures into atlas
        this.blockAtlas = new TextureAtlas(16, 16);
        loadBlockTextures(textureManager);
        
        // Create world shader
        try {
            this.worldShader = shaderManager.loadShader("chunk");
        } catch (Exception e) {
            LOGGER.error("Failed to load chunk shader", e);
            throw new RuntimeException("Failed to initialize world", e);
        }
        
        // Initialize chunk renderer
        this.chunkRenderer = new ChunkRenderer(camera, worldShader, blockAtlas, shaderManager);
        
        // Initialize chunk loader with texture lookup function
        this.chunkLoader = new ChunkLoader(config, terrainGenerator, chunkRenderer, this::getTextureLayer);
        
        LOGGER.info("World initialized successfully");
    }
    
    /**
     * Creates a 1x1 transparent texture for AIR block.
     */
    private Path createTransparentTexture() {
        try {
            Path tempFile = Files.createTempFile("air_texture", ".png");
            javax.imageio.ImageIO.setUseCache(false);
            tempFile.toFile().deleteOnExit();
            
            // Create a 1x1 transparent image using BufferedImage
            java.awt.image.BufferedImage img = new java.awt.image.BufferedImage(16, 16, java.awt.image.BufferedImage.TYPE_INT_ARGB);
            javax.imageio.ImageIO.write(img, "PNG", tempFile.toFile());
            return tempFile;
        } catch (HeadlessException | IOException e) {
            LOGGER.warn("Headless environment detected while creating AIR texture, using bundled fallback", e);
            try {
                Path fallback = loadTextureFromClasspath("textures/blocks/air.png");
                if (fallback != null) {
                    return fallback;
                }
            } catch (IOException ioException) {
                LOGGER.error("Failed to load bundled AIR texture fallback", ioException);
            }
            throw new RuntimeException("Failed to acquire AIR texture", e);
        }
    }
    
    /**
     * Loads a texture from classpath to a temporary file.
     */
    private Path loadTextureFromClasspath(String resourcePath) throws IOException {
        try (InputStream is = getClass().getClassLoader().getResourceAsStream(resourcePath)) {
            if (is == null) {
                return null;
            }

            String filename = resourcePath.substring(resourcePath.lastIndexOf('/') + 1);
            Path tempFile = Files.createTempFile("block_", "_" + filename);
            tempFile.toFile().deleteOnExit();

            Files.copy(is, tempFile, StandardCopyOption.REPLACE_EXISTING);
            return tempFile;
        }
    }
    
    /**
     * Loads block textures into the atlas.
     */
    private void loadBlockTextures(TextureManager textureManager) {
        try {
            // Reserve layer 0 for AIR - add a 1x1 transparent texture
            // Create a temporary 1x1 transparent PNG for AIR
            Path airTexturePath = createTransparentTexture();
            blockAtlas.addTexture("air", airTexturePath);
            
            // Load block textures from classpath
            addBlockTexture("stone", "air");
            addBlockTexture("dirt", "stone");
            addBlockTexture("grass_top", "stone");
            addBlockTexture("sand", "stone");
            addBlockTexture("sandstone", "stone");
            addBlockTexture("snow", "stone");
            addBlockTexture("ice", "stone");
            addBlockTexture("wood_top", "stone");
            addBlockTexture("leaves", "stone");
            addBlockTexture("water", "stone");

            // Additional textures for multi-face blocks
            addBlockTexture("grass_side", "stone");
            addBlockTexture("wood_side", "stone");
            
            blockAtlas.build();
            LOGGER.info("Loaded {} block textures", blockAtlas.getTextureCount());
        } catch (Exception e) {
            LOGGER.error("Failed to load block textures", e);
            throw new RuntimeException("Failed to load block textures", e);
        }
    }

    private void addBlockTexture(String textureName, String fallbackTextureName) throws IOException {
        Path texturePath = loadTextureOrFallback(textureName, fallbackTextureName);
        if (texturePath != null) {
            blockAtlas.addTexture(textureName, texturePath);
        }
    }

    private Path loadTextureOrFallback(String textureName, String fallbackTextureName) throws IOException {
        Path path = loadTextureFromClasspath("textures/blocks/" + textureName + ".png");
        if (path != null) {
            return path;
        }

        if (missingTextureWarnings.add(textureName)) {
            LOGGER.warn("Missing texture '{}', using fallback '{}'", textureName, fallbackTextureName);
        }

        if ("air".equals(fallbackTextureName)) {
            return createTransparentTexture();
        }

        Path fallbackPath = loadTextureFromClasspath("textures/blocks/" + fallbackTextureName + ".png");
        if (fallbackPath != null) {
            return fallbackPath;
        }

        if (!"air".equals(fallbackTextureName)) {
            return createTransparentTexture();
        }

        throw new IOException("Unable to locate fallback texture for " + textureName);
    }
    
    @Override
    public void update(double dt) {
        // Update chunk loading based on camera position
        chunkLoader.update(camera.getPosition().x, camera.getPosition().z);
        
        // Process chunks that need re-meshing after block edits
        java.util.List<ChunkPos> chunksToRemesh = chunkRenderer.getChunksNeedingRebuild(5);
        for (ChunkPos pos : chunksToRemesh) {
            chunkLoader.requestRemesh(pos.x(), pos.z());
        }
    }
    
    @Override
    public void render(double alpha) {
        // Update occlusion culling results before rendering
        chunkRenderer.updateOcclusionResults();
        
        // Render all visible chunks
        chunkRenderer.render(camera);
    }
    
    /**
     * Gets a block at world coordinates.
     * 
     * @param worldX World X coordinate
     * @param worldY World Y coordinate
     * @param worldZ World Z coordinate
     * @return Block ID
     */
    public int getBlock(int worldX, int worldY, int worldZ) {
        int chunkX = Math.floorDiv(worldX, 16);
        int chunkZ = Math.floorDiv(worldZ, 16);
        int localX = Math.floorMod(worldX, 16);
        int localZ = Math.floorMod(worldZ, 16);
        
        var chunk = chunkLoader.getChunk(chunkX, chunkZ);
        if (chunk != null) {
            return chunk.getBlock(localX, worldY, localZ);
        }
        return 0;
    }
    
    /**
     * Sets a block at world coordinates.
     * 
     * @param worldX World X coordinate
     * @param worldY World Y coordinate
     * @param worldZ World Z coordinate
     * @param blockId Block ID
     */
    public void setBlock(int worldX, int worldY, int worldZ, int blockId) {
        int chunkX = Math.floorDiv(worldX, 16);
        int chunkZ = Math.floorDiv(worldZ, 16);
        int localX = Math.floorMod(worldX, 16);
        int localZ = Math.floorMod(worldZ, 16);
        
        var chunk = chunkLoader.getChunk(chunkX, chunkZ);
        if (chunk != null) {
            chunk.setBlock(localX, worldY, localZ, blockId);
            chunkRenderer.markForRebuild(chunkX, chunkZ);
        }
    }
    
    /**
     * Cleans up world resources.
     */
    public void cleanup() {
        LOGGER.info("Cleaning up world");
        chunkLoader.shutdown();
        chunkRenderer.cleanup();
        blockAtlas.cleanup();
    }
    
    public long getSeed() {
        return seed;
    }
    
    /**
     * Gets the texture layer index for a given texture name.
     * 
     * @param textureName Name of the texture
     * @return Layer index, or 0 if not found
     */
    public int getTextureLayer(String textureName) {
        int layer = blockAtlas.getLayer(textureName);
        if (layer >= 0) {
            return layer;
        }

        if (missingTextureWarnings.add(textureName)) {
            LOGGER.warn("Texture '{}' not found in atlas, using AIR fallback", textureName);
        }

        return blockAtlas.getTextureIndex("air");
    }
}
