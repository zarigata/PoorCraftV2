package com.poorcraft.server.network.handler;

import com.poorcraft.common.network.ConnectionState;
import com.poorcraft.common.network.NetworkConnection;
import com.poorcraft.common.network.PacketHandler;
import com.poorcraft.common.network.packet.PlayerPositionPacket;
import com.poorcraft.common.network.packet.PlayerLookPacket;
import com.poorcraft.common.network.packet.PlayerPositionLookPacket;
import com.poorcraft.server.GameServer;
import com.poorcraft.server.PlayerSession;
import com.poorcraft.server.network.ServerConnection;
import org.slf4j.Logger;

/**
 * Handles player movement packets (position, look, position+look).
 * <p>
 * Validates movement data and updates player position/rotation.
 * Also handles anti-cheat validation and chunk streaming triggers.
 */
public class PlayerMovementHandler implements PacketHandler {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(PlayerMovementHandler.class);

    private final GameServer gameServer;

    public PlayerMovementHandler(GameServer gameServer) {
        this.gameServer = gameServer;
    }

    @Override
    public void handle(Object packet, NetworkConnection connection) {
        if (!(connection instanceof ServerConnection)) {
            LOGGER.error("PlayerMovementHandler received non-server connection");
            return;
        }

        ServerConnection serverConnection = (ServerConnection) connection;

        if (serverConnection.getConnectionState() != ConnectionState.PLAY) {
            LOGGER.warn("Ignoring movement from connection in state {}", serverConnection.getConnectionState());
            return;
        }

        PlayerSession session = serverConnection.getPlayerSession();
        if (session == null) {
            LOGGER.warn("Movement received from connection without session: {}", connection.getChannel().remoteAddress());
            return;
        }

        double x = 0, y = 0, z = 0;
        float yaw = 0, pitch = 0;
        boolean onGround = false;

        if (packet instanceof PlayerPositionPacket) {
            PlayerPositionPacket posPacket = (PlayerPositionPacket) packet;
            x = posPacket.getX();
            y = posPacket.getY();
            z = posPacket.getZ();
            onGround = posPacket.isOnGround();

            LOGGER.debug("Player position update from {}: ({}, {}, {}), onGround={}",
                connection.getChannel().remoteAddress(), x, y, z, onGround);

        } else if (packet instanceof PlayerLookPacket) {
            PlayerLookPacket lookPacket = (PlayerLookPacket) packet;
            yaw = lookPacket.getYaw();
            pitch = lookPacket.getPitch();
            onGround = lookPacket.isOnGround();

            LOGGER.debug("Player look update from {}: yaw={}, pitch={}, onGround={}",
                connection.getChannel().remoteAddress(), yaw, pitch, onGround);

        } else if (packet instanceof PlayerPositionLookPacket) {
            PlayerPositionLookPacket posLookPacket = (PlayerPositionLookPacket) packet;
            x = posLookPacket.getX();
            y = posLookPacket.getY();
            z = posLookPacket.getZ();
            yaw = posLookPacket.getYaw();
            pitch = posLookPacket.getPitch();
            onGround = posLookPacket.isOnGround();

            LOGGER.debug("Player position+look update from {}: ({}, {}, {}), yaw={}, pitch={}, onGround={}",
                connection.getChannel().remoteAddress(), x, y, z, yaw, pitch, onGround);
        }

        // Validate movement (basic anti-cheat)
        if (!validateMovement(x, y, z, yaw, pitch)) {
            LOGGER.warn("Invalid movement detected from {}", connection.getChannel().remoteAddress());
            serverConnection.disconnect("Invalid movement");
            return;
        }

        session.updatePosition(x, y, z, yaw, pitch, onGround);

        LOGGER.debug("Movement validated and processed for {}", connection.getChannel().remoteAddress());
    }

    /**
     * Validates movement data for anti-cheat purposes.
     *
     * @param x position X
     * @param y position Y
     * @param z position Z
     * @param yaw rotation yaw
     * @param pitch rotation pitch
     * @return true if movement is valid, false otherwise
     */
    private boolean validateMovement(double x, double y, double z, float yaw, float pitch) {
        // Basic validation - check for NaN and reasonable bounds
        if (Double.isNaN(x) || Double.isNaN(y) || Double.isNaN(z) ||
            Float.isNaN(yaw) || Float.isNaN(pitch)) {
            return false;
        }

        // Check reasonable world bounds (adjust as needed)
        if (Math.abs(x) > 30000000 || Math.abs(y) > 30000000 || Math.abs(z) > 30000000) {
            return false;
        }

        // Check reasonable rotation bounds
        if (Math.abs(yaw) > 360 || Math.abs(pitch) > 90) {
            return false;
        }

        // TODO: Add more sophisticated anti-cheat checks like speed limits, teleport detection, etc.

        return true;
    }
}
