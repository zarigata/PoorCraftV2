package com.poorcraft.common.network.packet;

import com.poorcraft.common.network.NetworkUtil;
import com.poorcraft.common.network.Packet;
import io.netty.buffer.ByteBuf;

/**
 * Player position packet sent by client.
 * <p>
 * Contains absolute position and on-ground status.
 */
public class PlayerPositionPacket implements Packet {
    private double x;
    private double y;
    private double z;
    private boolean onGround;

    /**
     * Default constructor for reading from buffer.
     */
    public PlayerPositionPacket() {}

    /**
     * Constructor for creating a player position packet.
     *
     * @param x position X
     * @param y position Y
     * @param z position Z
     * @param onGround whether the player is on ground
     */
    public PlayerPositionPacket(double x, double y, double z, boolean onGround) {
        this.x = x;
        this.y = y;
        this.z = z;
        this.onGround = onGround;
    }

    @Override
    public int getPacketId() {
        return 0x0C;
    }

    @Override
    public void write(ByteBuf buf) {
        buf.writeDouble(x);
        buf.writeDouble(y);
        buf.writeDouble(z);
        buf.writeBoolean(onGround);
    }

    @Override
    public void read(ByteBuf buf) {
        x = buf.readDouble();
        y = buf.readDouble();
        z = buf.readDouble();
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
     * Checks if the player is on ground.
     *
     * @return true if on ground
     */
    public boolean isOnGround() {
        return onGround;
    }
}
