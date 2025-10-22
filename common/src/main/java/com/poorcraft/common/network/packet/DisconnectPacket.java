package com.poorcraft.common.network.packet;

import com.poorcraft.common.network.NetworkUtil;
import com.poorcraft.common.network.Packet;
import io.netty.buffer.ByteBuf;

/**
 * Disconnect packet sent to forcibly close a connection.
 * <p>
 * Contains a reason message explaining why the connection was closed.
 */
public class DisconnectPacket implements Packet {
    private String reason;

    /**
     * Default constructor for reading from buffer.
     */
    public DisconnectPacket() {}

    /**
     * Constructor for creating a disconnect packet.
     *
     * @param reason the reason for disconnection
     */
    public DisconnectPacket(String reason) {
        this.reason = reason;
    }

    @Override
    public int getPacketId() {
        return 0x03;
    }

    @Override
    public void write(ByteBuf buf) {
        NetworkUtil.writeString(buf, reason);
    }

    @Override
    public void read(ByteBuf buf) {
        reason = NetworkUtil.readString(buf);
    }

    /**
     * Gets the disconnect reason.
     *
     * @return the reason message
     */
    public String getReason() {
        return reason;
    }
}
