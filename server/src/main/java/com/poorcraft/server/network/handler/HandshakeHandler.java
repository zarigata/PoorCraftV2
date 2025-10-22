package com.poorcraft.server.network.handler;

import com.poorcraft.common.Constants;
import com.poorcraft.common.network.ConnectionState;
import com.poorcraft.common.network.NetworkConnection;
import com.poorcraft.common.network.PacketHandler;
import com.poorcraft.common.network.packet.DisconnectPacket;
import com.poorcraft.common.network.packet.HandshakePacket;
import org.slf4j.Logger;

/**
 * Handles handshake packets from clients.
 * <p>
 * Validates protocol version and sets the next connection state.
 */
public class HandshakeHandler implements PacketHandler<HandshakePacket> {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(HandshakeHandler.class);

    @Override
    public void handle(HandshakePacket packet, NetworkConnection connection) {
        // Validate protocol version
        if (packet.getProtocolVersion() != Constants.Game.PROTOCOL_VERSION) {
            LOGGER.warn("Client {} has incompatible protocol version: {} (expected: {})",
                connection.getChannel().remoteAddress(),
                packet.getProtocolVersion(),
                Constants.Game.PROTOCOL_VERSION);

            String reason = String.format("Incompatible protocol version. Server: %d, Client: %d",
                Constants.Game.PROTOCOL_VERSION, packet.getProtocolVersion());
            connection.sendPacket(new DisconnectPacket(reason));
            connection.disconnect(reason);
            return;
        }

        // Set next state based on handshake
        ConnectionState nextState;
        switch (packet.getNextState()) {
            case 1:
                nextState = ConnectionState.LOGIN;
                break;
            case 2:
                nextState = ConnectionState.PLAY;
                break;
            default:
                LOGGER.warn("Client {} requested invalid next state: {}",
                    connection.getChannel().remoteAddress(), packet.getNextState());
                connection.sendPacket(new DisconnectPacket("Invalid next state"));
                connection.disconnect("Invalid next state");
                return;
        }

        connection.setConnectionState(nextState);
        LOGGER.debug("Handshake completed for {}, transitioning to {}",
            connection.getChannel().remoteAddress(), nextState);
    }
}
