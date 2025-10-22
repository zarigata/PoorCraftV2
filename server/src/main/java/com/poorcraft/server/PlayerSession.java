package com.poorcraft.server;

import com.poorcraft.common.entity.EntityType;
import com.poorcraft.common.network.packet.ChunkDataPacket;
import com.poorcraft.common.world.block.BlockType;
import com.poorcraft.common.world.chunk.Chunk;
import com.poorcraft.common.world.chunk.ChunkCodec;
import com.poorcraft.server.network.ServerConnection;
import com.poorcraft.server.world.ServerEntity;
import com.poorcraft.server.world.ServerWorld;
import org.slf4j.Logger;

import java.util.HashSet;
import java.util.Set;
import java.util.UUID;

/**
 * Represents a player's session on the server.
 * <p>
 * Manages the player's loaded chunks, view distance, and sends
 * relevant updates to keep the client synchronized.
 */
public class PlayerSession {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(PlayerSession.class);

    private final ServerConnection connection;
    private final UUID playerUuid;
    private final ServerWorld world;
    private final String username;

    private volatile double pendingX;
    private volatile double pendingY;
    private volatile double pendingZ;
    private volatile float pendingYaw;
    private volatile float pendingPitch;
    private volatile boolean pendingOnGround;

    private double x, y, z; // Player position
    private float yaw, pitch; // Player rotation
    private boolean onGround;
    private int viewDistance = 8; // Default view distance
    private final Set<Long> loadedChunks = new HashSet<>();
    private volatile boolean positionDirty;
    private volatile boolean chunkRefreshRequested = true;

    private ServerEntity playerEntity;
    private int playerEntityId = -1;

    /**
     * Creates a new player session.
     *
     * @param connection the player's network connection
     * @param playerUuid the player's UUID
     * @param world the server world
     * @param username the player's username
     */
    public PlayerSession(ServerConnection connection, UUID playerUuid, ServerWorld world, String username) {
        this.connection = connection;
        this.playerUuid = playerUuid;
        this.world = world;
        this.username = username;
    }

    /**
     * Updates the player's position and rotation.
     *
     * @param x position X
     * @param y position Y
     * @param z position Z
     * @param yaw rotation yaw
     * @param pitch rotation pitch
     * @param onGround whether the player is on ground
     */
    public void updatePosition(double x, double y, double z, float yaw, float pitch, boolean onGround) {
        synchronized (this) {
            this.pendingX = x;
            this.pendingY = y;
            this.pendingZ = z;
            this.pendingYaw = yaw;
            this.pendingPitch = pitch;
            this.pendingOnGround = onGround;
            this.positionDirty = true;
        }

        LOGGER.trace("Queued position update for {}: ({}, {}, {}), yaw={}, pitch={} onGround={}",
            playerUuid, x, y, z, yaw, pitch, onGround);
    }

    /**
     * Performs per-tick maintenance for this session.
     */
    public void tick() {
        applyPendingTransform();

        if (chunkRefreshRequested) {
            updateChunkLoading();
            chunkRefreshRequested = false;
        }
    }

    /**
     * Updates chunk loading based on current position and view distance.
     */
    private void updateChunkLoading() {
        int chunkX = (int) Math.floor(x / 16);
        int chunkZ = (int) Math.floor(z / 16);

        Set<Long> requiredChunks = new HashSet<>();

        // Calculate which chunks should be loaded based on view distance
        for (int dx = -viewDistance; dx <= viewDistance; dx++) {
            for (int dz = -viewDistance; dz <= viewDistance; dz++) {
                if (Math.abs(dx) + Math.abs(dz) <= viewDistance) {
                    int cx = chunkX + dx;
                    int cz = chunkZ + dz;
                    long chunkPos = packChunkPosition(cx, cz);
                    requiredChunks.add(chunkPos);
                }
            }
        }

        // Determine chunks to unload
        Set<Long> toUnload = new HashSet<>(loadedChunks);
        toUnload.removeAll(requiredChunks);

        for (Long chunkPos : toUnload) {
            unloadChunk(chunkPos);
            loadedChunks.remove(chunkPos);
        }

        // Determine chunks to load
        Set<Long> toLoad = new HashSet<>(requiredChunks);
        toLoad.removeAll(loadedChunks);

        for (Long chunkPos : toLoad) {
            loadChunk(chunkPos);
            loadedChunks.add(chunkPos);
        }
    }

    /**
     * Loads a chunk and sends it to the client.
     *
     * @param chunkPos the packed chunk position
     */
    private void loadChunk(long chunkPos) {
        int chunkX = unpackChunkX(chunkPos);
        int chunkZ = unpackChunkZ(chunkPos);

        Chunk chunk = world.getOrCreateChunk(chunkX, chunkZ);
        if (chunk == null) {
            LOGGER.warn("Unable to stream chunk ({}, {}) for player {}", chunkX, chunkZ, playerUuid);
            return;
        }

        byte[] chunkData = ChunkCodec.encodeFullChunk(chunk);

        ChunkDataPacket packet = new ChunkDataPacket(chunkX, chunkZ, true, chunkData);

        connection.sendPacket(packet);

        LOGGER.debug("Loaded chunk ({}, {}) for player {}", chunkX, chunkZ, playerUuid);
    }

    /**
     * Unloads a chunk from the client.
     *
     * @param chunkPos the packed chunk position
     */
    private void unloadChunk(long chunkPos) {
        // TODO: Send chunk unload packet when protocol supports it
        LOGGER.debug("Unloaded chunk {} for player {}", chunkPos, playerUuid);
    }

    /**
     * Sets the view distance for this player.
     *
     * @param viewDistance the new view distance
     */
    public void setViewDistance(int viewDistance) {
        this.viewDistance = Math.max(2, Math.min(32, viewDistance));
        this.chunkRefreshRequested = true;
    }

    /**
     * Gets the current view distance.
     *
     * @return the view distance
     */
    public int getViewDistance() {
        return viewDistance;
    }

    /**
     * Gets the player's UUID.
     *
     * @return the UUID
     */
    public UUID getPlayerUuid() {
        return playerUuid;
    }

    /**
     * Gets the player's connection.
     *
     * @return the connection
     */
    public ServerConnection getConnection() {
        return connection;
    }

    /**
     * Gets the player's current position X.
     *
     * @return the X coordinate
     */
    public double getX() {
        return x;
    }

    /**
     * Gets the player's current position Y.
     *
     * @return the Y coordinate
     */
    public double getY() {
        return y;
    }

    /**
     * Gets the player's current position Z.
     *
     * @return the Z coordinate
     */
    public double getZ() {
        return z;
    }

    /**
     * Gets the player's yaw rotation.
     *
     * @return the yaw
     */
    public float getYaw() {
        return yaw;
    }

    /**
     * Gets the player's pitch rotation.
     *
     * @return the pitch
     */
    public float getPitch() {
        return pitch;
    }

    public boolean isOnGround() {
        return onGround;
    }

    public String getUsername() {
        return username;
    }

    public ServerWorld getWorld() {
        return world;
    }

    public ServerEntity getPlayerEntity() {
        return playerEntity;
    }

    public void setPlayerEntity(ServerEntity playerEntity) {
        this.playerEntity = playerEntity;
        if (playerEntity != null) {
            this.playerEntityId = playerEntity.getEntityId();
            playerEntity.setPosition(x, y, z, yaw, pitch, onGround);
        }
    }

    public int getPlayerEntityId() {
        return playerEntityId;
    }

    public int getPlacementBlockId() {
        // TODO: derive from inventory
        return BlockType.STONE.getId();
    }

    public void initializePosition(double initialX, double initialY, double initialZ) {
        synchronized (this) {
            this.x = initialX;
            this.y = initialY;
            this.z = initialZ;
            this.yaw = 0.0f;
            this.pitch = 0.0f;
            this.onGround = false;

            this.pendingX = initialX;
            this.pendingY = initialY;
            this.pendingZ = initialZ;
            this.pendingYaw = 0.0f;
            this.pendingPitch = 0.0f;
            this.pendingOnGround = false;
            this.positionDirty = false;
        }
        this.chunkRefreshRequested = true;
    }

    public void requestChunkRefresh() {
        this.chunkRefreshRequested = true;
    }

    /**
     * Called when the player disconnects.
     * <p>
     * Cleans up session state.
     */
    public void disconnect() {
        LOGGER.info("Player session ended for {}", playerUuid);

        // TODO: Save player data
        // TODO: Remove from world
        loadedChunks.clear();
    }

    /**
     * Packs chunk coordinates into a long.
     *
     * @param chunkX the chunk X coordinate
     * @param chunkZ the chunk Z coordinate
     * @return the packed position
     */
    private long packChunkPosition(int chunkX, int chunkZ) {
        return ((long) chunkX << 32) | (chunkZ & 0xFFFFFFFFL);
    }

    /**
     * Unpacks the X coordinate from a packed chunk position.
     *
     * @param chunkPos the packed position
     * @return the X coordinate
     */
    private int unpackChunkX(long chunkPos) {
        return (int) (chunkPos >> 32);
    }

    /**
     * Unpacks the Z coordinate from a packed chunk position.
     *
     * @param chunkPos the packed position
     * @return the Z coordinate
     */
    private int unpackChunkZ(long chunkPos) {
        return (int) (chunkPos & 0xFFFFFFFFL);
    }

    private void applyPendingTransform() {
        if (!positionDirty) {
            return;
        }

        double newX;
        double newY;
        double newZ;
        float newYaw;
        float newPitch;
        boolean newOnGround;

        synchronized (this) {
            if (!positionDirty) {
                return;
            }
            newX = pendingX;
            newY = pendingY;
            newZ = pendingZ;
            newYaw = pendingYaw;
            newPitch = pendingPitch;
            newOnGround = pendingOnGround;
            positionDirty = false;
        }

        this.x = newX;
        this.y = newY;
        this.z = newZ;
        this.yaw = newYaw;
        this.pitch = newPitch;
        this.onGround = newOnGround;

        if (playerEntity == null) {
            chunkRefreshRequested = true;
            return;
        }

        playerEntity.setPosition(newX, newY, newZ, newYaw, newPitch, newOnGround);
        chunkRefreshRequested = true;
    }
}
