package com.poorcraft.common.network.packet;

import com.poorcraft.common.network.NetworkUtil;
import com.poorcraft.common.network.Packet;
import io.netty.buffer.ByteBuf;

/**
 * Player position and look packet sent by client.
 * <p>
 * Contains both position and rotation in a single packet.
 */
public class PlayerPositionLookPacket implements Packet {
    private double x;
    private double y;
    private double z;
    private float yaw;
    private float pitch;
    private boolean onGround;

    /**
     * Default constructor for reading from buffer.
     */
    public PlayerPositionLookPacket() {}

    /**
     * Constructor for creating a player position and look packet.
     *
     * @param x position X
     * @param y position Y
     * @param z position Z
     * @param yaw horizontal rotation
     * @param pitch vertical rotation
     * @param onGround whether the player is on ground
     */
    public PlayerPositionLookPacket(double x, double y, double z, float yaw, float pitch, boolean onGround) {
        this.x = x;
        this.y = y;
        this.z = z;
        this.yaw = yaw;
        this.pitch = pitch;
        this.onGround = onGround;
    }

    @Override
    public int getPacketId() {
        return 0x0E;
    }

    @Override
    public void write(ByteBuf buf) {
        buf.writeDouble(x);
        buf.writeDouble(y);
        buf.writeDouble(z);
        buf.writeFloat(yaw);
        buf.writeFloat(pitch);
        buf.writeBoolean(onGround);
    }

    @Override
    public void read(ByteBuf buf) {
        x = buf.readDouble();
        y = buf.readDouble();
        z = buf.readDouble();
        yaw = buf.readFloat();
        pitch = buf.readFloat();
        onGround = buf.readBoolean();
    }

    /**
     * Gets the position X.
     *
     * @return the X coordinate
     */
    public double getX() {
        return x;
    }

    /**
     * Gets the position Y.
     *
     * @return the Y coordinate
     */
    public double getY() {
        return y;
    }

    /**
     * Gets the position Z.
     *
     * @return the Z coordinate
     */
    public double getZ() {
        return z;
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
