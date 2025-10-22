package com.poorcraft.server.network.handler;

import com.poorcraft.common.network.ConnectionState;
import com.poorcraft.common.network.NetworkConnection;
import com.poorcraft.common.network.PacketHandler;
import com.poorcraft.common.network.packet.LoginStartPacket;
import com.poorcraft.common.network.packet.LoginSuccessPacket;
import com.poorcraft.common.network.packet.ModListPacket;
import com.poorcraft.server.GameServer;
import com.poorcraft.server.network.ServerConnection;
import org.slf4j.Logger;

import java.util.UUID;

/**
 * Handles login start packets from clients.
 * <p>
 * Validates username, generates UUID, sends login success, and creates player session.
 */
public class LoginHandler implements PacketHandler<LoginStartPacket> {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(LoginHandler.class);

    private final GameServer gameServer;

    /**
     * Creates a new login handler.
     *
     * @param gameServer the game server for session management
     */
    public LoginHandler(GameServer gameServer) {
        this.gameServer = gameServer;
    }

    @Override
    public void handle(LoginStartPacket packet, NetworkConnection connection) {
        if (!(connection instanceof ServerConnection)) {
            LOGGER.error("LoginHandler received non-server connection");
            return;
        }

        ServerConnection serverConnection = (ServerConnection) connection;

        // Validate username
        String username = packet.getUsername();
        if (username == null || username.trim().isEmpty()) {
            serverConnection.sendPacket(new com.poorcraft.common.network.packet.DisconnectPacket("Invalid username"));
            serverConnection.disconnect("Invalid username");
            return;
        }

        if (username.length() > 16) {
            serverConnection.sendPacket(new com.poorcraft.common.network.packet.DisconnectPacket("Username too long"));
            serverConnection.disconnect("Username too long");
            return;
        }

        // Generate UUID (in a real implementation, this might be looked up from a database)
        UUID playerUuid = UUID.nameUUIDFromBytes(("OfflinePlayer:" + username).getBytes());

        // Send login success
        serverConnection.sendPacket(new LoginSuccessPacket(playerUuid, username));

        // Send required mod list before transitioning to play state
        ModListPacket modListPacket = new ModListPacket(gameServer.getModManager().getModList());
        serverConnection.sendPacket(modListPacket);

        // Transition to PLAY state
        connection.setConnectionState(ConnectionState.PLAY);

        // Attach UUID to connection
        serverConnection.setPlayerUuid(playerUuid);

        // Create session through game server
        gameServer.handlePlayerJoin(serverConnection, playerUuid, username);

        LOGGER.info("Player {} ({}) logged in from {}",
            username, playerUuid, connection.getChannel().remoteAddress());
    }
}
