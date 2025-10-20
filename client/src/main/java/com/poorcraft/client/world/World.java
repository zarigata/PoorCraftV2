package com.poorcraft.client.world;

import com.poorcraft.client.render.camera.Camera;
import com.poorcraft.client.render.shader.ShaderManager;
import com.poorcraft.client.render.shader.ShaderProgram;
import com.poorcraft.client.render.texture.TextureAtlas;
import com.poorcraft.client.render.texture.TextureManager;
import com.poorcraft.common.config.Configuration;
import com.poorcraft.common.world.gen.TerrainGenerator;
import com.poorcraft.core.Renderable;
import com.poorcraft.core.Updatable;
import org.slf4j.Logger;

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
            this.worldShader = shaderManager.loadShader("chunk", "shaders/chunk.vert", "shaders/chunk.frag");
        } catch (Exception e) {
            LOGGER.error("Failed to load chunk shader", e);
            throw new RuntimeException("Failed to initialize world", e);
        }
        
        // Initialize chunk renderer
        this.chunkRenderer = new ChunkRenderer(camera, worldShader, blockAtlas);
        
        // Initialize chunk loader
        this.chunkLoader = new ChunkLoader(config, terrainGenerator, chunkRenderer);
        
        LOGGER.info("World initialized successfully");
    }
    
    /**
     * Loads block textures into the atlas.
     */
    private void loadBlockTextures(TextureManager textureManager) {
        try {
            // Load textures in order matching block IDs
            blockAtlas.addTexture("air", null); // 0 - Air (no texture)
            blockAtlas.addTexture("stone", textureManager.loadTexture("textures/blocks/stone.png"));
            blockAtlas.addTexture("dirt", textureManager.loadTexture("textures/blocks/dirt.png"));
            blockAtlas.addTexture("grass_top", textureManager.loadTexture("textures/blocks/grass_top.png"));
            blockAtlas.addTexture("sand", textureManager.loadTexture("textures/blocks/sand.png"));
            blockAtlas.addTexture("sandstone", textureManager.loadTexture("textures/blocks/sandstone.png"));
            blockAtlas.addTexture("snow", textureManager.loadTexture("textures/blocks/snow.png"));
            blockAtlas.addTexture("ice", textureManager.loadTexture("textures/blocks/ice.png"));
            blockAtlas.addTexture("wood_top", textureManager.loadTexture("textures/blocks/wood_top.png"));
            blockAtlas.addTexture("leaves", textureManager.loadTexture("textures/blocks/leaves.png"));
            blockAtlas.addTexture("water", textureManager.loadTexture("textures/blocks/water.png"));
            
            // Additional textures for multi-face blocks
            blockAtlas.addTexture("grass_side", textureManager.loadTexture("textures/blocks/grass_side.png"));
            blockAtlas.addTexture("wood_side", textureManager.loadTexture("textures/blocks/wood_side.png"));
            
            blockAtlas.build();
            LOGGER.info("Loaded {} block textures", blockAtlas.getTextureCount());
        } catch (Exception e) {
            LOGGER.error("Failed to load block textures", e);
            throw new RuntimeException("Failed to load block textures", e);
        }
    }
    
    @Override
    public void update(double dt) {
        // Update chunk loading based on camera position
        chunkLoader.update(camera.getPosition().x, camera.getPosition().z);
        
        // Update chunk renderer
        chunkRenderer.update(5); // Max 5 rebuilds per frame
    }
    
    @Override
    public void render(double alpha) {
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
        return blockAtlas.getTextureIndex(textureName);
    }
}
