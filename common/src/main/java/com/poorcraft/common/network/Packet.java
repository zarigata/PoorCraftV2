package com.poorcraft.common.network;

import io.netty.buffer.ByteBuf;

/**
 * Base interface for all network packets.
 * <p>
 * Packets are used for communication between client and server.
 * Each packet has a unique ID and can be serialized to/from a ByteBuf.
 */
public interface Packet {
    /**
     * Gets the unique packet ID for this packet type.
     * <p>
     * IDs are used for packet routing and should be unique across all packet types.
     * Serverbound and clientbound packets share the same ID space but are handled separately.
     *
     * @return the packet ID
     */
    int getPacketId();

    /**
     * Writes this packet's data to the given ByteBuf.
     * <p>
     * Implementations should write all necessary data in a consistent order.
     * Use {@link NetworkUtil} for common serialization operations.
     *
     * @param buf the buffer to write to
     */
    void write(ByteBuf buf);

    /**
     * Reads this packet's data from the given ByteBuf.
     * <p>
     * Implementations should read data in the same order as written in {@link #write(ByteBuf)}.
     * Use {@link NetworkUtil} for common deserialization operations.
     * <p>
     * This method should validate buffer bounds and throw appropriate exceptions on errors.
     *
     * @param buf the buffer to read from
     * @throws IllegalArgumentException if the buffer data is invalid
     */
    void read(ByteBuf buf);
}
