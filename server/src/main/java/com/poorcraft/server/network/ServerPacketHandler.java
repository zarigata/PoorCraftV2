package com.poorcraft.server.network;

import com.poorcraft.common.network.ConnectionState;
import com.poorcraft.common.network.Packet;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.SimpleChannelInboundHandler;
import io.netty.util.AttributeKey;
import org.slf4j.Logger;

/**
 * Main handler for server-side packet processing.
 * <p>
 * Manages connection lifecycle, dispatches packets to registered handlers,
 * and updates connection state.
 */
public class ServerPacketHandler extends SimpleChannelInboundHandler<Packet> {
    static final AttributeKey<ConnectionState> CONNECTION_STATE_KEY =
        AttributeKey.valueOf("connection_state");
    static final AttributeKey<ServerConnection> CONNECTION_KEY =
        AttributeKey.valueOf("server_connection");

    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(ServerPacketHandler.class);

    private final ServerNetworkManager networkManager;

    /**
     * Creates a new server packet handler.
     *
     * @param networkManager the server network manager
     */
    public ServerPacketHandler(ServerNetworkManager networkManager) {
        this.networkManager = networkManager;
    }

    @Override
    public void channelActive(ChannelHandlerContext ctx) {
        // Create new server connection
        ServerConnection connection = new ServerConnection(ctx.channel(), networkManager);

        // Set initial state to HANDSHAKE
        ctx.channel().attr(CONNECTION_STATE_KEY).set(ConnectionState.HANDSHAKE);
        ctx.channel().attr(CONNECTION_KEY).set(connection);

        // Notify network manager
        networkManager.onConnectionEstablished(ctx.channel(), connection);

        LOGGER.debug("Connection established: {}", ctx.channel().remoteAddress());
    }

    @Override
    public void channelInactive(ChannelHandlerContext ctx) {
        // Notify network manager
        networkManager.onConnectionClosed(ctx.channel());

        LOGGER.debug("Connection closed: {}", ctx.channel().remoteAddress());
    }

    @Override
    protected void channelRead0(ChannelHandlerContext ctx, Packet packet) {
        ServerConnection connection = ctx.channel().attr(CONNECTION_KEY).get();
        if (connection == null) {
            LOGGER.warn("Received packet but no connection object: {}", ctx.channel().remoteAddress());
            ctx.close();
            return;
        }

        try {
            // Update last packet time
            connection.updateLastPacketTime();

            // Dispatch to handler
            networkManager.handlePacket(packet, connection);

        } catch (Exception e) {
            LOGGER.error("Error handling packet {} from {}",
                packet.getClass().getSimpleName(), ctx.channel().remoteAddress(), e);
            ctx.close();
        }
    }

    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) {
        LOGGER.error("Exception in server packet handler for {}",
            ctx.channel().remoteAddress(), cause);
        ctx.close();
    }
}
