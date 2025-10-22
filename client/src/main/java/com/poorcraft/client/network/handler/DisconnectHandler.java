package com.poorcraft.client.network.handler;

import com.poorcraft.common.network.NetworkConnection;
import com.poorcraft.common.network.PacketHandler;
import com.poorcraft.common.network.packet.DisconnectPacket;
import com.poorcraft.client.network.ClientConnection;
import org.slf4j.Logger;

/**
 * Handles disconnect packets from server.
 * <p>
 * Logs the disconnect reason and notifies the game.
 */
public class DisconnectHandler implements PacketHandler<DisconnectPacket> {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(DisconnectHandler.class);

    @Override
    public void handle(DisconnectPacket packet, NetworkConnection connection) {
        if (!(connection instanceof ClientConnection)) {
            LOGGER.error("DisconnectHandler received non-client connection");
            return;
        }

        LOGGER.info("Disconnected from server: {}", packet.getReason());

        // TODO: Show disconnect screen to user
        // TODO: Return to main menu
        // TODO: Clean up world state

        // Close connection
        connection.disconnect(packet.getReason());
    }
}
