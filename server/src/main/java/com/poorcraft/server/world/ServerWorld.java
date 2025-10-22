package com.poorcraft.server.world;

import com.poorcraft.api.event.block.BlockBreakEvent;
import com.poorcraft.api.event.block.BlockPlaceEvent;
import com.poorcraft.api.event.entity.EntitySpawnEvent;
import com.poorcraft.api.event.entity.EntitySpawnEvent.SpawnReason;
import com.poorcraft.common.Constants;
import com.poorcraft.common.config.Configuration;
import com.poorcraft.common.event.EventBus;
import com.poorcraft.common.inventory.ItemStack;
import com.poorcraft.common.network.packet.BlockUpdatePacket;
import com.poorcraft.common.network.packet.ChunkDataPacket;
import com.poorcraft.common.network.packet.EntitySpawnPacket;
import com.poorcraft.common.network.packet.EntityPositionPacket;
import com.poorcraft.common.network.packet.EntityVelocityPacket;
import com.poorcraft.common.network.packet.EntityRemovePacket;
import com.poorcraft.common.network.packet.MultiBlockUpdatePacket;
import com.poorcraft.common.world.World;
import com.poorcraft.common.world.block.BlockType;
import com.poorcraft.common.world.chunk.Chunk;
import com.poorcraft.common.world.chunk.ChunkCodec;
import com.poorcraft.common.world.gen.TerrainGenerator;
import com.poorcraft.common.entity.EntityType;
import com.poorcraft.server.GameServer;
import com.poorcraft.server.PlayerSession;
import com.poorcraft.server.network.ServerConnection;
import com.poorcraft.server.network.ServerNetworkManager;
import org.slf4j.Logger;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Server-side world implementation with authoritative state.
 * <p>
 * Manages chunks, entities, block updates, and broadcasts changes to clients.
 */
public class ServerWorld extends World {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(ServerWorld.class);

    private final Configuration config;
    private volatile GameServer gameServer;
    private volatile ServerNetworkManager networkManager;
    private EventBus eventBus;
    private final Map<UUID, PlayerSession> playerSessions = new ConcurrentHashMap<>();

    private final Map<Long, Chunk> chunks = new ConcurrentHashMap<>();
    private final Map<Integer, ServerEntity> entities = new ConcurrentHashMap<>();
    private final TerrainGenerator terrainGenerator;

    private int nextEntityId = 1;

    /**
     * Creates a new server world.
     *
     * @param seed the world seed
     * @param config the server configuration
     */
    public ServerWorld(long seed, Configuration config) {
        super(seed, config);
        this.config = config;
        this.gameServer = null; // Will be set after construction
        this.networkManager = null; // Will be set after construction
        this.terrainGenerator = new TerrainGenerator(seed, config);
    }

    /**
     * Sets the game server reference (called after construction).
     *
     * @param gameServer the game server
     */
    public void setGameServer(GameServer gameServer) {
        this.gameServer = gameServer;
        this.networkManager = gameServer.getNetworkManager();
        this.eventBus = gameServer.getEventBus();
    }

    @Override
    public void init() {
        LOGGER.info("Initializing server world with seed: {}", getSeed());

        // TODO: Load world from disk or generate new world
        // For now, just initialize basic state

        LOGGER.info("Server world initialized");
    }

    @Override
    public void tick() {
        // Update entities
        for (ServerEntity entity : entities.values()) {
            entity.tick();

            // Broadcast position updates if needed
            if (entity.hasPositionChanged()) {
                broadcastEntityPosition(entity);
            }
        }

        // TODO: Update chunks, weather, time, etc.
    }

    @Override
    public void save() {
        LOGGER.info("Saving server world...");

        // TODO: Save world state to disk

        LOGGER.info("Server world saved");
    }

    /**
     * Gets the block state at the given position.
     *
     * @param x the X coordinate
     * @param y the Y coordinate
     * @param z the Z coordinate
     * @return the block ID, or 0 if not set
     */
    @Override
    public int getBlock(int x, int y, int z) {
        if (y < 0 || y >= Constants.World.CHUNK_SIZE_Y) {
            return 0;
        }

        int chunkX = Math.floorDiv(x, 16);
        int chunkZ = Math.floorDiv(z, 16);
        Chunk chunk = getOrCreateChunk(chunkX, chunkZ);
        if (chunk == null) {
            return 0;
        }

        int localX = Math.floorMod(x, 16);
        int localZ = Math.floorMod(z, 16);
        return chunk.getBlock(localX, y, localZ);
    }

    /**
     * Sets the block state at the given position.
     *
     * @param x the X coordinate
     * @param y the Y coordinate
     * @param z the Z coordinate
     * @param blockId the new block ID
     */
    @Override
    public void setBlock(int x, int y, int z, int blockId) {
        if (y < 0 || y >= Constants.World.CHUNK_SIZE_Y) {
            return;
        }

        int chunkX = Math.floorDiv(x, 16);
        int chunkZ = Math.floorDiv(z, 16);
        Chunk chunk = getOrCreateChunk(chunkX, chunkZ);
        if (chunk == null) {
            return;
        }

        int localX = Math.floorMod(x, 16);
        int localZ = Math.floorMod(z, 16);
        int currentBlockId = chunk.getBlock(localX, y, localZ);

        if (currentBlockId == blockId) {
            return;
        }

        if (eventBus != null) {
            if (blockId == BlockType.AIR.getId()) {
                BlockBreakEvent event = new BlockBreakEvent(this, x, y, z, currentBlockId, null, Collections.emptyList());
                eventBus.post(event);
                if (event.isCancelled()) {
                    return;
                }
                blockId = BlockType.AIR.getId();
            } else {
                BlockPlaceEvent event = new BlockPlaceEvent(this, x, y, z, blockId, null, ItemStack.EMPTY);
                eventBus.post(event);
                if (event.isCancelled()) {
                    return;
                }
                blockId = event.getBlockId();
            }
        }

        chunk.setBlock(localX, y, localZ, blockId);

        // Broadcast update to all clients
        broadcastBlockUpdate(x, y, z, blockId);
    }

    /**
     * Spawns an entity in the world.
     *
     * @param entity the entity to spawn
     * @return the entity ID
     */
    public int spawnEntity(ServerEntity entity) {
        return spawnEntity(entity, SpawnReason.UNKNOWN);
    }

    public int spawnEntity(ServerEntity entity, SpawnReason reason) {
        int entityId = nextEntityId++;
        entity.setEntityId(entityId);
        if (eventBus != null) {
            EntitySpawnEvent event = new EntitySpawnEvent(null, this, reason);
            eventBus.post(event);
            if (event.isCancelled()) {
                return -1;
            }
        }

        entities.put(entityId, entity);

        // Broadcast spawn to all clients
        broadcastEntitySpawn(entity);

        LOGGER.debug("Spawned entity {} with ID {}", entity.getClass().getSimpleName(), entityId);
        return entityId;
    }

    /**
     * Removes an entity from the world.
     *
     * @param entityId the entity ID
     */
    public void removeEntity(int entityId) {
        ServerEntity entity = entities.remove(entityId);
        if (entity != null) {
            // Broadcast removal to all clients
            broadcastEntityRemove(entityId);

            LOGGER.debug("Removed entity with ID {}", entityId);
        }
    }

    /**
     * Gets a player session by UUID.
     *
     * @param playerUuid the player UUID
     * @return the player session, or null if not found
     */
    public PlayerSession getPlayerSession(UUID playerUuid) {
        return playerSessions.get(playerUuid);
    }

    /**
     * Creates a new player session.
     *
     * @param connection the player connection
     * @param playerUuid the player UUID
     * @return the new player session
     */
    public PlayerSession createPlayerSession(ServerConnection connection, UUID playerUuid, String username) {
        PlayerSession session = new PlayerSession(connection, playerUuid, this, username);
        playerSessions.put(playerUuid, session);

        LOGGER.info("Created player session for {}", playerUuid);
        return session;
    }

    /**
     * Removes a player session.
     *
     * @param playerUuid the player UUID
     */
    public void removePlayerSession(UUID playerUuid) {
        PlayerSession session = playerSessions.remove(playerUuid);
        if (session != null) {
            LOGGER.info("Removed player session for {}", playerUuid);
        }
    }

    /**
     * Broadcasts a block update to all clients.
     *
     * @param x the block X coordinate
     * @param y the block Y coordinate
     * @param z the block Z coordinate
     * @param blockId the new block ID
     */
    private void broadcastBlockUpdate(int x, int y, int z, int blockId) {
        if (networkManager == null) {
            return;
        }
        BlockUpdatePacket packet = new BlockUpdatePacket(x, y, z, blockId);
        networkManager.broadcast(packet);
    }

    /**
     * Broadcasts an entity spawn to all clients.
     *
     * @param entity the entity that spawned
     */
    private void broadcastEntitySpawn(ServerEntity entity) {
        if (networkManager == null) {
            return;
        }
        EntitySpawnPacket packet = new EntitySpawnPacket(
            entity.getEntityId(),
            entity.getUuid(),
            entity.getType(),
            entity.getX(), entity.getY(), entity.getZ(),
            entity.getYaw(), entity.getPitch(),
            entity.getVelocityX(), entity.getVelocityY(), entity.getVelocityZ()
        );
        networkManager.broadcast(packet);
    }

    /**
     * Broadcasts an entity position update to all clients.
     *
     * @param entity the entity that moved
     */
    private void broadcastEntityPosition(ServerEntity entity) {
        if (networkManager == null) {
            return;
        }
        EntityPositionPacket packet = new EntityPositionPacket(
            entity.getEntityId(),
            entity.getDeltaX(), entity.getDeltaY(), entity.getDeltaZ(),
            entity.isOnGround()
        );
        networkManager.broadcast(packet);

        // Reset delta tracking
        entity.resetPositionChanged();
    }

    /**
     * Broadcasts an entity removal to all clients.
     *
     * @param entityId the entity ID
     */
    private void broadcastEntityRemove(int entityId) {
        if (networkManager == null) {
            return;
        }
        EntityRemovePacket packet = new EntityRemovePacket(new int[]{entityId});
        networkManager.broadcast(packet);
    }

    /**
     * Gets the number of active player sessions.
     *
     * @return the player count
     */
    public int getPlayerCount() {
        return playerSessions.size();
    }
}
