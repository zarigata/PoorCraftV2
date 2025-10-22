package com.poorcraft.client.network.handler;

import com.poorcraft.common.network.NetworkConnection;
import com.poorcraft.common.network.PacketHandler;
import com.poorcraft.common.network.packet.KeepAlivePacket;
import com.poorcraft.client.network.ClientConnection;
import org.slf4j.Logger;

/**
 * Handles keep-alive packets from server.
 * <p>
 * Responds immediately with the same keep-alive ID.
 */
public class KeepAliveHandler implements PacketHandler<KeepAlivePacket> {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(KeepAliveHandler.class);

    @Override
    public void handle(KeepAlivePacket packet, NetworkConnection connection) {
        if (!(connection instanceof ClientConnection)) {
            LOGGER.error("KeepAliveHandler received non-client connection");
            return;
        }

        ClientConnection clientConnection = (ClientConnection) connection;

        // Update latency tracking
        clientConnection.handleKeepAlive(packet.getKeepAliveId());

        // Echo back immediately
        clientConnection.sendPacket(packet);

        LOGGER.debug("Processed keep-alive packet with ID {}", packet.getKeepAliveId());
    }
}
