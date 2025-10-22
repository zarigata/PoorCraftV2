package com.poorcraft.client.world;

import com.poorcraft.client.network.ClientNetworkManager;
import com.poorcraft.api.ModAPI;
import com.poorcraft.api.event.block.BlockBreakEvent;
import com.poorcraft.api.event.block.BlockPlaceEvent;
import com.poorcraft.client.network.handler.BlockUpdateHandler;
import com.poorcraft.client.network.handler.ChunkDataHandler;
import com.poorcraft.client.network.handler.DisconnectHandler;
import com.poorcraft.client.network.handler.EntityUpdateHandler;
import com.poorcraft.client.network.handler.KeepAliveHandler;
import com.poorcraft.client.network.handler.LoginSuccessHandler;
import com.poorcraft.common.event.EventBus;
import com.poorcraft.common.inventory.ItemStack;
import com.poorcraft.common.world.block.BlockType;
import com.poorcraft.common.world.chunk.Chunk;
import com.poorcraft.common.world.chunk.ChunkCodec;

import java.awt.HeadlessException;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

/**
 * Main World class that coordinates all world systems.
 */
public class World implements Updatable, Renderable, com.poorcraft.common.world.World {
    
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
    private final EntityManager entityManager;
    private final EntityRenderer entityRenderer;
    private final InputManager inputManager;
    private final BlockSelectionRenderer blockSelectionRenderer;
    private final ClientNetworkManager networkManager;
    private final EventBus eventBus;
    private Entity playerEntity;
    private PlayerController playerController;
    private BlockInteractionHandler blockInteractionHandler;
    
    /**
     * Creates a new world.
     * 
     * @param seed World seed
     * @param config Configuration
     * @param camera Camera for rendering
     * @param shaderManager Shader manager
     * @param textureManager Texture manager
     * @param inputManager Input manager
     * @param networkManager Network manager (can be null for singleplayer)
     */
    public World(long seed, Configuration config, Camera camera,
                 ShaderManager shaderManager, TextureManager textureManager,
                 InputManager inputManager, ClientNetworkManager networkManager) {
        this.seed = seed;
        this.config = config;
        this.camera = camera;
        this.inputManager = inputManager;
        this.networkManager = networkManager;
        this.eventBus = ModAPI.getEventBus();
        
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
        
        this.entityManager = new EntityManager(config);
        this.entityRenderer = new EntityRenderer(entityManager, camera, shaderManager, textureManager, config);
        this.blockSelectionRenderer = new BlockSelectionRenderer(camera, shaderManager);

        // Initialize network manager if provided
        if (networkManager != null) {
            networkManager.init();
            registerNetworkHandlers(networkManager);
            LOGGER.info("Network manager initialized for multiplayer");
        }

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

        // Update networking if enabled
        if (networkManager != null) {
            networkManager.tick();
        }

        entityManager.update(dt);

        if (playerController != null) {
            playerController.update((float) dt);
        }
        if (blockInteractionHandler != null) {
            blockInteractionHandler.update((float) dt);
        }
    }

    @Override
    public void render(double alpha) {
        // Update occlusion culling results before rendering
        chunkRenderer.updateOcclusionResults();
        
        // Render all visible chunks
        chunkRenderer.render(camera);

        entityRenderer.render(alpha);

        if (blockInteractionHandler != null && playerEntity != null) {
            blockSelectionRenderer.render(
                    blockInteractionHandler.getCurrentTarget(),
                    playerEntity.getComponent(com.poorcraft.common.entity.component.InteractionComponent.class).getBreakProgress()
            );
        }
    }
    
    /**
     * Gets a block at world coordinates.
     * 
     * @param worldX World X coordinate
     * @param worldY World Y coordinate
     * @param worldZ World Z coordinate
     * @return Block ID
     */
    @Override
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
    @Override
    public void setBlock(int worldX, int worldY, int worldZ, int blockId) {
        int chunkX = Math.floorDiv(worldX, 16);
        int chunkZ = Math.floorDiv(worldZ, 16);
        int localX = Math.floorMod(worldX, 16);
        int localZ = Math.floorMod(worldZ, 16);

        Chunk chunk = chunkLoader.getChunk(chunkX, chunkZ);
        if (chunk == null) {
            return;
        }

        int currentBlockId = chunk.getBlock(localX, worldY, localZ);
        if (currentBlockId == blockId) {
            return;
        }

        if (eventBus != null) {
            boolean singleplayer = networkManager == null;
            if (blockId == BlockType.AIR.getId()) {
                BlockBreakEvent event = new BlockBreakEvent(this, worldX, worldY, worldZ, currentBlockId, null, Collections.emptyList());
                eventBus.post(event);
                if (singleplayer && event.isCancelled()) {
                    return;
                }
                blockId = BlockType.AIR.getId();
            } else {
                BlockPlaceEvent event = new BlockPlaceEvent(this, worldX, worldY, worldZ, blockId, null, ItemStack.EMPTY);
                eventBus.post(event);
                if (singleplayer) {
                    if (event.isCancelled()) {
                        return;
                    }
                    blockId = event.getBlockId();
                }
            }
        }

        chunk.setBlock(localX, worldY, localZ, blockId);
        chunkRenderer.markForRebuild(chunkX, chunkZ);
    }

    public void addNetworkChunk(int chunkX, int chunkZ, byte[] data) {
        try {
            Chunk chunk = ChunkCodec.decodeFullChunk(data);
            if (chunk.getChunkX() != chunkX || chunk.getChunkZ() != chunkZ) {
                LOGGER.warn("Chunk payload coordinates ({}, {}) did not match header ({}, {})",
                    chunk.getChunkX(), chunk.getChunkZ(), chunkX, chunkZ);
            }
            chunkLoader.putChunk(chunk);
            chunkRenderer.markForRebuild(chunk.getChunkX(), chunk.getChunkZ());
        } catch (Exception e) {
            LOGGER.error("Failed to install network chunk ({}, {})", chunkX, chunkZ, e);
        }
    }
    
    /**
     * Registers network packet handlers.
     *
     * @param networkManager the network manager
     */
    private void registerNetworkHandlers(ClientNetworkManager networkManager) {
        EntityUpdateHandler entityHandler = new EntityUpdateHandler(this, entityManager);

        networkManager.registerHandler(com.poorcraft.common.network.packet.LoginSuccessPacket.class,
            new LoginSuccessHandler());
        networkManager.registerHandler(com.poorcraft.common.network.packet.ChunkDataPacket.class,
            new ChunkDataHandler(this));
        networkManager.registerHandler(com.poorcraft.common.network.packet.BlockUpdatePacket.class,
            new BlockUpdateHandler(this));
        networkManager.registerHandler(com.poorcraft.common.network.packet.EntitySpawnPacket.class, entityHandler);
        networkManager.registerHandler(com.poorcraft.common.network.packet.EntityPositionPacket.class, entityHandler);
        networkManager.registerHandler(com.poorcraft.common.network.packet.EntityVelocityPacket.class, entityHandler);
        networkManager.registerHandler(com.poorcraft.common.network.packet.EntityRemovePacket.class, entityHandler);
        networkManager.registerHandler(com.poorcraft.common.network.packet.KeepAlivePacket.class,
            new KeepAliveHandler());
        networkManager.registerHandler(com.poorcraft.common.network.packet.DisconnectPacket.class,
            new DisconnectHandler());
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

    public EntityManager getEntityManager() {
        return entityManager;
    }

    public Entity spawnPlayer(String name, Vector3f position) {
        Entity entity = entityManager.createPlayer(name, new Vector3f(position));
        entity.setWorld(this);
        this.playerEntity = entity;
        this.playerController = new PlayerController(entity, camera, inputManager, config, networkManager);
        this.blockInteractionHandler = new BlockInteractionHandler(entity, this, camera, inputManager, config, networkManager);
        return entity;
    }

    public Entity spawnNPC(String name, Vector3f position, String behavior) {
        Entity entity = entityManager.createNPC(name, new Vector3f(position), behavior);
        entity.setWorld(this);
        return entity;
    }
}
