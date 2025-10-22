package com.poorcraft.common.network;

import io.netty.channel.Channel;

/**
 * Abstraction for a network connection.
 * <p>
 * Represents a connection between client and server, providing access to
 * the underlying Netty channel, connection state, and methods for sending packets.
 * Implementations should handle connection lifecycle and packet transmission.
 */
public interface NetworkConnection {
    /**
     * Gets the underlying Netty channel for this connection.
     *
     * @return the channel
     */
    Channel getChannel();

    /**
     * Gets the current connection state.
     *
     * @return the connection state
     */
    ConnectionState getConnectionState();

    /**
     * Sets the connection state.
     *
     * @param state the new state
     */
    void setConnectionState(ConnectionState state);

    /**
     * Sends a packet to the remote endpoint.
     * <p>
     * The packet will be encoded and transmitted asynchronously.
     *
     * @param packet the packet to send
     */
    void sendPacket(Packet packet);

    /**
     * Disconnects this connection with an optional reason.
     * <p>
     * After calling this method, the connection should no longer be used.
     *
     * @param reason optional reason for disconnection (may be null)
     */
    void disconnect(String reason);

    /**
     * Checks if this connection is still active.
     *
     * @return true if the connection is active, false otherwise
     */
    boolean isConnected();

    /**
     * Gets the timestamp of the last received packet.
     *
     * @return the last packet timestamp in milliseconds since epoch
     */
    long getLastPacketTime();

    /**
     * Updates the last packet timestamp to the current time.
     */
    void updateLastPacketTime();

    /**
     * Gets the latency of this connection in milliseconds.
     * <p>
     * Latency is calculated from keep-alive packets or other timing mechanisms.
     *
     * @return the latency in milliseconds, or -1 if not available
     */
    int getLatency();
}
