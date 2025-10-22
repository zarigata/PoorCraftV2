package com.poorcraft.server.network.handler;

import com.poorcraft.common.network.ConnectionState;
import com.poorcraft.common.network.NetworkConnection;
import com.poorcraft.common.network.PacketHandler;
import com.poorcraft.common.network.packet.PlayerBlockPlacementPacket;
import com.poorcraft.common.network.packet.PlayerDiggingPacket;
import com.poorcraft.common.world.block.BlockType;
import com.poorcraft.common.entity.EntityType;
import com.poorcraft.server.PlayerSession;
import com.poorcraft.server.network.ServerConnection;
import com.poorcraft.server.world.ServerWorld;
import org.slf4j.Logger;

/**
 * Handles block interaction packets (digging and placement).
 * <p>
 * Validates permissions, reach distance, and processes block changes.
 * Broadcasts updates to all clients.
 */
public class BlockInteractionHandler implements PacketHandler {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(BlockInteractionHandler.class);
    private static final double MAX_REACH_SQUARED = 25.0;

    @Override
    public void handle(Object packet, NetworkConnection connection) {
        if (!(connection instanceof ServerConnection)) {
            LOGGER.error("BlockInteractionHandler received non-server connection");
            return;
        }

        ServerConnection serverConnection = (ServerConnection) connection;

        if (packet instanceof PlayerDiggingPacket) {
            handleDigging((PlayerDiggingPacket) packet, serverConnection);
        } else if (packet instanceof PlayerBlockPlacementPacket) {
            handlePlacement((PlayerBlockPlacementPacket) packet, serverConnection);
        }
    }

    /**
     * Handles player digging (block breaking).
     *
     * @param packet the digging packet
     * @param connection the player connection
     */
    private void handleDigging(PlayerDiggingPacket packet, ServerConnection connection) {
        if (connection.getConnectionState() != ConnectionState.PLAY) {
            LOGGER.warn("Ignoring digging from connection in state {}", connection.getConnectionState());
            return;
        }

        PlayerSession session = connection.getPlayerSession();
        if (session == null) {
            LOGGER.warn("Ignoring digging from {} without session", connection.getChannel().remoteAddress());
            return;
        }

        ServerWorld world = session.getWorld();
        if (world == null) {
            LOGGER.warn("Ignoring digging for session {} without world", session.getPlayerUuid());
            return;
        }

        int action = packet.getAction();
        int x = packet.getX();
        int y = packet.getY();
        int z = packet.getZ();

        LOGGER.debug("Player digging at ({}, {}, {}) with action {} from {}",
            x, y, z, action, connection.getChannel().remoteAddress());

        // Validate reach distance
        if (!validateReach(session, x, y, z)) {
            LOGGER.warn("Player digging out of reach at ({}, {}, {}) from {}",
                x, y, z, connection.getChannel().remoteAddress());
            return;
        }

        // TODO: Check if player has permission to break blocks at this location
        // TODO: Apply block breaking time based on tool and block type
        // TODO: Handle inventory updates (drop items, etc.)

        switch (action) {
            case 0: // Start digging
                LOGGER.debug("Starting to dig block at ({}, {}, {})", x, y, z);
                break;
            case 1: // Finish digging
                LOGGER.debug("Finished digging block at ({}, {}, {})", x, y, z);
                int currentBlock = world.getBlock(x, y, z);
                if (currentBlock == BlockType.AIR.getId()) {
                    LOGGER.debug("Skipping removal at ({}, {}, {}) as block is already air", x, y, z);
                    break;
                }
                world.setBlock(x, y, z, BlockType.AIR.getId());
                break;
            case 2: // Cancel digging
                LOGGER.debug("Cancelled digging at ({}, {}, {})", x, y, z);
                break;
        }
    }

    /**
     * Handles player block placement.
     *
     * @param packet the placement packet
     * @param connection the player connection
     */
    private void handlePlacement(PlayerBlockPlacementPacket packet, ServerConnection connection) {
        if (connection.getConnectionState() != ConnectionState.PLAY) {
            LOGGER.warn("Ignoring placement from connection in state {}", connection.getConnectionState());
            return;
        }

        PlayerSession session = connection.getPlayerSession();
        if (session == null) {
            LOGGER.warn("Ignoring placement from {} without session", connection.getChannel().remoteAddress());
            return;
        }

        ServerWorld world = session.getWorld();
        if (world == null) {
            LOGGER.warn("Ignoring placement for session {} without world", session.getPlayerUuid());
            return;
        }

        int x = packet.getX();
        int y = packet.getY();
        int z = packet.getZ();
        int face = packet.getFace();

        int placeX = x;
        int placeY = y;
        int placeZ = z;

        switch (face) {
            case 0:
                placeY -= 1;
                break;
            case 1:
                placeY += 1;
                break;
            case 2:
                placeZ -= 1;
                break;
            case 3:
                placeZ += 1;
                break;
            case 4:
                placeX -= 1;
                break;
            case 5:
                placeX += 1;
                break;
            default:
                LOGGER.warn("Invalid placement face {} from {}", face, connection.getChannel().remoteAddress());
                return;
        }

        LOGGER.debug("Player placing block at ({}, {}, {}) face {} from {}",
            x, y, z, face, connection.getChannel().remoteAddress());

        // Validate reach distance
        if (!validateReach(session, placeX, placeY, placeZ)) {
            LOGGER.warn("Player placing block out of reach at ({}, {}, {}) from {}",
                placeX, placeY, placeZ, connection.getChannel().remoteAddress());
            return;
        }

        if (isPlacementColliding(session, placeX, placeY, placeZ)) {
            LOGGER.warn("Player attempted placement intersecting own bounding box at ({}, {}, {})", placeX, placeY, placeZ);
            return;
        }

        // TODO: Check if player has permission to place blocks at this location
        // TODO: Check if the target location is valid for placement
        // TODO: Check if player has the block in inventory
        // TODO: Handle inventory updates

        if (world.getBlock(placeX, placeY, placeZ) != BlockType.AIR.getId()) {
            LOGGER.debug("Block placement rejected at ({}, {}, {}) as block is not air", placeX, placeY, placeZ);
            return;
        }

        int blockId = session.getPlacementBlockId();
        if (blockId == BlockType.AIR.getId()) {
            LOGGER.debug("Block placement rejected at ({}, {}, {}) due to invalid block id", placeX, placeY, placeZ);
            return;
        }

        world.setBlock(placeX, placeY, placeZ, blockId);
        LOGGER.debug("Block placement validated at ({}, {}, {})", placeX, placeY, placeZ);
    }

    /**
     * Validates that the player can reach the given block position.
     *
     * @param connection the player connection
     * @param x block X coordinate
     * @param y block Y coordinate
     * @param z block Z coordinate
     * @return true if reachable, false otherwise
     */
    private boolean validateReach(PlayerSession session, int x, int y, int z) {
        double dx = (x + 0.5) - session.getX();
        double dy = (y + 0.5) - session.getY();
        double dz = (z + 0.5) - session.getZ();
        double distanceSquared = dx * dx + dy * dy + dz * dz;
        return distanceSquared <= MAX_REACH_SQUARED;
    }

    private boolean isPlacementColliding(PlayerSession session, int x, int y, int z) {
        double halfWidth = EntityType.PLAYER.getWidth() / 2.0;
        double playerMinX = session.getX() - halfWidth;
        double playerMaxX = session.getX() + halfWidth;
        double playerMinY = session.getY();
        double playerMaxY = session.getY() + EntityType.PLAYER.getHeight();
        double playerMinZ = session.getZ() - halfWidth;
        double playerMaxZ = session.getZ() + halfWidth;

        double blockMinX = x;
        double blockMaxX = x + 1;
        double blockMinY = y;
        double blockMaxY = y + 1;
        double blockMinZ = z;
        double blockMaxZ = z + 1;

        boolean intersectX = playerMaxX > blockMinX && playerMinX < blockMaxX;
        boolean intersectY = playerMaxY > blockMinY && playerMinY < blockMaxY;
        boolean intersectZ = playerMaxZ > blockMinZ && playerMinZ < blockMaxZ;
        return intersectX && intersectY && intersectZ;
    }
}
