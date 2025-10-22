package com.poorcraft.common.network.packet;

import com.poorcraft.common.network.NetworkUtil;
import com.poorcraft.common.network.Packet;
import io.netty.buffer.ByteBuf;

/**
 * Login start packet sent by client to begin authentication.
 * <p>
 * Contains the player's username for authentication.
 */
public class LoginStartPacket implements Packet {
    private String username;

    /**
     * Default constructor for reading from buffer.
     */
    public LoginStartPacket() {}

    /**
     * Constructor for creating a login start packet.
     *
     * @param username the player's username
     */
    public LoginStartPacket(String username) {
        this.username = username;
    }

    @Override
    public int getPacketId() {
        return 0x01;
    }

    @Override
    public void write(ByteBuf buf) {
        NetworkUtil.writeString(buf, username);
    }

    @Override
    public void read(ByteBuf buf) {
        username = NetworkUtil.readString(buf);
    }

    /**
     * Gets the username.
     *
     * @return the username
     */
    public String getUsername() {
        return username;
    }
}
