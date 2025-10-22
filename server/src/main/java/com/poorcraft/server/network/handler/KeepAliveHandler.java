package com.poorcraft.server.network.handler;

import com.poorcraft.common.network.NetworkConnection;
import com.poorcraft.common.network.PacketHandler;
import com.poorcraft.common.network.packet.KeepAlivePacket;
import com.poorcraft.server.network.ServerConnection;
import org.slf4j.Logger;

/**
 * Handles keep-alive packets from clients.
 * <p>
 * Updates connection latency and last packet time.
 */
public class KeepAliveHandler implements PacketHandler<KeepAlivePacket> {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(KeepAliveHandler.class);

    @Override
    public void handle(KeepAlivePacket packet, NetworkConnection connection) {
        if (!(connection instanceof ServerConnection)) {
            LOGGER.error("KeepAliveHandler received non-server connection");
            return;
        }

        ServerConnection serverConnection = (ServerConnection) connection;

        // Update last packet time
        serverConnection.updateLastPacketTime();

        // Handle keep-alive response
        serverConnection.handleKeepAlive(packet.getKeepAliveId());

        LOGGER.debug("Keep-alive response received from {} with latency {}ms",
            connection.getChannel().remoteAddress(), serverConnection.getLatency());
    }
}
