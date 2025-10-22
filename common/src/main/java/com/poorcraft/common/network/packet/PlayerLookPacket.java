package com.poorcraft.common.network.packet;

import com.poorcraft.common.network.NetworkUtil;
import com.poorcraft.common.network.Packet;
import io.netty.buffer.ByteBuf;

/**
 * Player look packet sent by client.
 * <p>
 * Contains rotation angles and on-ground status.
 */
public class PlayerLookPacket implements Packet {
    private float yaw;
    private float pitch;
    private boolean onGround;

    /**
     * Default constructor for reading from buffer.
     */
    public PlayerLookPacket() {}

    /**
     * Constructor for creating a player look packet.
     *
     * @param yaw horizontal rotation
     * @param pitch vertical rotation
     * @param onGround whether the player is on ground
     */
    public PlayerLookPacket(float yaw, float pitch, boolean onGround) {
        this.yaw = yaw;
        this.pitch = pitch;
        this.onGround = onGround;
    }

    @Override
    public int getPacketId() {
        return 0x0D;
    }

    @Override
    public void write(ByteBuf buf) {
        buf.writeFloat(yaw);
        buf.writeFloat(pitch);
        buf.writeBoolean(onGround);
    }

    @Override
    public void read(ByteBuf buf) {
        yaw = buf.readFloat();
        pitch = buf.readFloat();
        onGround = buf.readBoolean();
    }

    /**
     * Gets the yaw rotation.
     *
     * @return the yaw
     */
    public float getYaw() {
        return yaw;
    }

    /**
     * Gets the pitch rotation.
     *
     * @return the pitch
     */
    public float getPitch() {
        return pitch;
    }

    /**
     * Checks if the player is on ground.
     *
     * @return true if on ground
     */
    public boolean isOnGround() {
        return onGround;
    }
}
