package com.poorcraft.client.network;

import com.poorcraft.common.network.ConnectionState;
import com.poorcraft.common.network.NetworkConnection;
import com.poorcraft.common.network.Packet;
import com.poorcraft.common.network.packet.KeepAlivePacket;
import io.netty.channel.Channel;
import io.netty.channel.ChannelFuture;
import io.netty.channel.ChannelFutureListener;
import org.slf4j.Logger;

import java.util.UUID;
import java.util.concurrent.atomic.AtomicLong;

/**
 * Client-side network connection wrapper.
 * <p>
 * Manages connection state, latency tracking, keep-alive handling,
 * and provides methods for sending packets and disconnecting.
 */
public class ClientConnection implements NetworkConnection {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(ClientConnection.class);

    private final Channel channel;
    private final ClientNetworkManager networkManager;
    private final AtomicLong lastKeepAliveId = new AtomicLong(0);
    private final AtomicLong sentKeepAliveId = new AtomicLong(0);

    private ConnectionState connectionState = ConnectionState.HANDSHAKE;
    private long lastPacketTime = System.currentTimeMillis();
    private int latency = -1;
    private UUID playerUuid;

    /**
     * Creates a new client connection.
     *
     * @param channel the Netty channel
     * @param networkManager the client network manager
     */
    public ClientConnection(Channel channel, ClientNetworkManager networkManager) {
        this.channel = channel;
        this.networkManager = networkManager;
    }

    @Override
    public Channel getChannel() {
        return channel;
    }

    @Override
    public ConnectionState getConnectionState() {
        return connectionState;
    }

    @Override
    public void setConnectionState(ConnectionState state) {
        this.connectionState = state;
        LOGGER.debug("Connection state changed to: {}", state);
    }

    @Override
    public void sendPacket(Packet packet) {
        if (channel.isActive()) {
            channel.writeAndFlush(packet).addListener((ChannelFutureListener) future -> {
                if (!future.isSuccess()) {
                    LOGGER.error("Failed to send packet {} to {}",
                        packet.getClass().getSimpleName(), channel.remoteAddress(), future.cause());
                }
            });
        }
    }

    @Override
    public void disconnect(String reason) {
        if (channel.isActive()) {
            if (reason != null) {
                // Send disconnect packet first
                Packet disconnectPacket = new com.poorcraft.common.network.packet.DisconnectPacket(reason);
                sendPacket(disconnectPacket);
            }

            channel.close();
        }
    }

    @Override
    public boolean isConnected() {
        return channel.isActive();
    }

    @Override
    public long getLastPacketTime() {
        return lastPacketTime;
    }

    @Override
    public void updateLastPacketTime() {
        this.lastPacketTime = System.currentTimeMillis();
    }

    @Override
    public int getLatency() {
        return latency;
    }

    /**
     * Sets the player UUID for this connection.
     *
     * @param playerUuid the player UUID
     */
    public void setPlayerUuid(UUID playerUuid) {
        this.playerUuid = playerUuid;
    }

    /**
     * Gets the player UUID for this connection.
     *
     * @return the player UUID, or null if not authenticated
     */
    public UUID getPlayerUuid() {
        return playerUuid;
    }

    /**
     * Handles a keep-alive response.
     *
     * @param keepAliveId the keep-alive ID from the response
     */
    public void handleKeepAlive(long keepAliveId) {
        long sentId = sentKeepAliveId.get();
        if (keepAliveId == sentId) {
            long now = System.currentTimeMillis();
            latency = (int) (now - (lastKeepAliveId.get() >> 32));
            LOGGER.debug("Keep-alive response received, latency: {}ms", latency);
        } else {
            LOGGER.warn("Received keep-alive response with unexpected ID: {} (expected: {})",
                keepAliveId, sentId);
        }
    }

    /**
     * Sends a keep-alive packet.
     *
     * @return true if a keep-alive was sent, false otherwise
     */
    public boolean sendKeepAlive() {
        long keepAliveId = System.nanoTime(); // Simple ID generation
        lastKeepAliveId.set((keepAliveId << 32) | System.currentTimeMillis());
        sentKeepAliveId.set(keepAliveId);

        sendPacket(new KeepAlivePacket(keepAliveId));
        return true;
    }

    /**
     * Performs periodic maintenance tasks.
     * <p>
     * Should be called regularly from the main game loop.
     */
    public void tick() {
        // TODO: Check for keep-alive timeout and send if needed
        // For now, just update last packet time
        updateLastPacketTime();
    }
}
