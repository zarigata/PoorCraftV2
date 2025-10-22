package com.poorcraft.common.network.packet;

import com.poorcraft.common.network.NetworkUtil;
import com.poorcraft.common.network.Packet;
import io.netty.buffer.ByteBuf;

import java.util.UUID;

/**
 * Login success packet sent by server to confirm successful authentication.
 * <p>
 * Contains the player's UUID and username, and transitions the connection to PLAY state.
 */
public class LoginSuccessPacket implements Packet {
    private UUID uuid;
    private String username;

    /**
     * Default constructor for reading from buffer.
     */
    public LoginSuccessPacket() {}

    /**
     * Constructor for creating a login success packet.
     *
     * @param uuid the player's UUID
     * @param username the player's username
     */
    public LoginSuccessPacket(UUID uuid, String username) {
        this.uuid = uuid;
        this.username = username;
    }

    @Override
    public int getPacketId() {
        return 0x02;
    }

    @Override
    public void write(ByteBuf buf) {
        NetworkUtil.writeUUID(buf, uuid);
        NetworkUtil.writeString(buf, username);
    }

    @Override
    public void read(ByteBuf buf) {
        uuid = NetworkUtil.readUUID(buf);
        username = NetworkUtil.readString(buf);
    }

    /**
     * Gets the player's UUID.
     *
     * @return the UUID
     */
    public UUID getUuid() {
        return uuid;
    }

    /**
     * Gets the player's username.
     *
     * @return the username
     */
    public String getUsername() {
        return username;
    }
}
