package com.poorcraft.common.network.packet;

import com.poorcraft.common.network.NetworkUtil;
import com.poorcraft.common.network.Packet;
import io.netty.buffer.ByteBuf;

/**
 * Keep-alive packet for connection health monitoring.
 * <p>
 * Sent periodically to ensure the connection is still active and to measure latency.
 */
public class KeepAlivePacket implements Packet {
    private long keepAliveId;

    /**
     * Default constructor for reading from buffer.
     */
    public KeepAlivePacket() {}

    /**
     * Constructor for creating a keep-alive packet.
     *
     * @param keepAliveId unique identifier for this keep-alive
     */
    public KeepAlivePacket(long keepAliveId) {
        this.keepAliveId = keepAliveId;
    }

    @Override
    public int getPacketId() {
        return 0x04;
    }

    @Override
    public void write(ByteBuf buf) {
        NetworkUtil.writeVarInt(buf, (int) keepAliveId);
    }

    @Override
    public void read(ByteBuf buf) {
        keepAliveId = NetworkUtil.readVarInt(buf);
    }

    /**
     * Gets the keep-alive ID.
     *
     * @return the keep-alive ID
     */
    public long getKeepAliveId() {
        return keepAliveId;
    }
}
