package com.poorcraft.client.network;

import com.poorcraft.common.network.ConnectionState;
import com.poorcraft.common.network.Packet;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.SimpleChannelInboundHandler;
import io.netty.util.AttributeKey;
import org.slf4j.Logger;

/**
 * Main handler for client-side packet processing.
 * <p>
 * Dispatches packets to registered handlers and manages connection state.
 */
public class ClientPacketHandler extends SimpleChannelInboundHandler<Packet> {
    static final AttributeKey<ConnectionState> CONNECTION_STATE_KEY =
        AttributeKey.valueOf("connection_state");
    static final AttributeKey<ClientConnection> CONNECTION_KEY =
        AttributeKey.valueOf("client_connection");

    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(ClientPacketHandler.class);

    private final ClientNetworkManager networkManager;

    /**
     * Creates a new client packet handler.
     *
     * @param networkManager the client network manager
     */
    public ClientPacketHandler(ClientNetworkManager networkManager) {
        this.networkManager = networkManager;
    }

    @Override
    public void channelActive(ChannelHandlerContext ctx) {
        // Create connection wrapper
        ClientConnection connection = new ClientConnection(ctx.channel(), networkManager);

        // Set initial state to HANDSHAKE
        ctx.channel().attr(CONNECTION_STATE_KEY).set(ConnectionState.HANDSHAKE);
        ctx.channel().attr(CONNECTION_KEY).set(connection);

        LOGGER.debug("Connection established: {}", ctx.channel().remoteAddress());
    }

    @Override
    public void channelInactive(ChannelHandlerContext ctx) {
        LOGGER.debug("Connection closed: {}", ctx.channel().remoteAddress());

        // TODO: Notify game of disconnection
    }

    @Override
    protected void channelRead0(ChannelHandlerContext ctx, Packet packet) {
        ClientConnection connection = ctx.channel().attr(CONNECTION_KEY).get();
        if (connection == null) {
            LOGGER.warn("Received packet but no connection object: {}", ctx.channel().remoteAddress());
            ctx.close();
            return;
        }

        try {
            // Update last packet time
            connection.updateLastPacketTime();

            // Dispatch to handler on main thread
            networkManager.enqueue(() -> networkManager.handlePacket(packet, connection));

        } catch (Exception e) {
            LOGGER.error("Error handling packet {} from {}",
                packet.getClass().getSimpleName(), ctx.channel().remoteAddress(), e);
            ctx.close();
        }
    }

    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) {
        LOGGER.error("Exception in client packet handler for {}",
            ctx.channel().remoteAddress(), cause);
        ctx.close();
    }
}
