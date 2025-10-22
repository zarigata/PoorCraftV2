package com.poorcraft.common.network.packet;

import com.poorcraft.common.network.NetworkUtil;
import com.poorcraft.common.network.Packet;
import io.netty.buffer.ByteBuf;

/**
 * Chat message packet for sending chat messages.
 * <p>
 * Used for both sending and receiving chat messages.
 */
public class ChatMessagePacket implements Packet {
    private String message;
    private int type;

    /**
     * Default constructor for reading from buffer.
     */
    public ChatMessagePacket() {}

    /**
     * Constructor for creating a chat message packet.
     *
     * @param message the chat message
     * @param type the message type (0=chat, 1=system, 2=action)
     */
    public ChatMessagePacket(String message, int type) {
        this.message = message;
        this.type = type;
    }

    @Override
    public int getPacketId() {
        return 0x12;
    }

    @Override
    public void write(ByteBuf buf) {
        NetworkUtil.writeString(buf, message);
        buf.writeByte(type);
    }

    @Override
    public void read(ByteBuf buf) {
        message = NetworkUtil.readString(buf);
        type = buf.readByte();
    }

    /**
     * Gets the chat message.
     *
     * @return the message
     */
    public String getMessage() {
        return message;
    }

    /**
     * Gets the message type.
     *
     * @return the type (0=chat, 1=system, 2=action)
     */
    public int getType() {
        return type;
    }
}
