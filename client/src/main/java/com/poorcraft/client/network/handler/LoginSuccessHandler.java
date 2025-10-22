package com.poorcraft.client.network.handler;

import com.poorcraft.common.network.ConnectionState;
import com.poorcraft.common.network.NetworkConnection;
import com.poorcraft.common.network.PacketHandler;
import com.poorcraft.common.network.packet.LoginSuccessPacket;
import com.poorcraft.client.network.ClientConnection;
import org.slf4j.Logger;

/**
 * Handles login success packets from server.
 * <p>
 * Transitions connection to PLAY state and notifies game of successful login.
 */
public class LoginSuccessHandler implements PacketHandler<LoginSuccessPacket> {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(LoginSuccessHandler.class);

    @Override
    public void handle(LoginSuccessPacket packet, NetworkConnection connection) {
        if (!(connection instanceof ClientConnection)) {
            LOGGER.error("LoginSuccessHandler received non-client connection");
            return;
        }

        ClientConnection clientConnection = (ClientConnection) connection;

        // Set player UUID
        clientConnection.setPlayerUuid(packet.getUuid());

        // Transition to PLAY state
        connection.setConnectionState(ConnectionState.PLAY);

        LOGGER.info("Login successful for player: {} ({})",
            packet.getUsername(), packet.getUuid());

        // TODO: Notify game of successful login
        // TODO: Initialize world, player, etc.
        // TODO: Send initial position/look packets
    }
}
