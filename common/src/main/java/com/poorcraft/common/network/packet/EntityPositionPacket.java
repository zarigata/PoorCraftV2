package com.poorcraft.common.network.packet;

import com.poorcraft.common.network.NetworkUtil;
import com.poorcraft.common.network.Packet;
import io.netty.buffer.ByteBuf;

/**
 * Entity position update packet for relative position changes.
 * <p>
 * Sent to update entity positions with delta encoding for efficiency.
 */
public class EntityPositionPacket implements Packet {
    private int entityId;
    private double deltaX;
    private double deltaY;
    private double deltaZ;
    private boolean onGround;

    /**
     * Default constructor for reading from buffer.
     */
    public EntityPositionPacket() {}

    /**
     * Constructor for creating an entity position packet.
     *
     * @param entityId the entity ID
     * @param deltaX change in X position
     * @param deltaY change in Y position
     * @param deltaZ change in Z position
     * @param onGround whether the entity is on ground
     */
    public EntityPositionPacket(int entityId, double deltaX, double deltaY, double deltaZ, boolean onGround) {
        this.entityId = entityId;
        this.deltaX = deltaX;
        this.deltaY = deltaY;
        this.deltaZ = deltaZ;
        this.onGround = onGround;
    }

    @Override
    public int getPacketId() {
        return 0x09;
    }

    @Override
    public void write(ByteBuf buf) {
        NetworkUtil.writeVarInt(buf, entityId);
        buf.writeShort((short) (deltaX * 4096.0));
        buf.writeShort((short) (deltaY * 4096.0));
        buf.writeShort((short) (deltaZ * 4096.0));
        buf.writeBoolean(onGround);
    }

    @Override
    public void read(ByteBuf buf) {
        entityId = NetworkUtil.readVarInt(buf);
        deltaX = buf.readShort() / 4096.0;
        deltaY = buf.readShort() / 4096.0;
        deltaZ = buf.readShort() / 4096.0;
        onGround = buf.readBoolean();
    }

    /**
     * Gets the entity ID.
     *
     * @return the entity ID
     */
    public int getEntityId() {
        return entityId;
    }

    /**
     * Gets the delta X.
     *
     * @return the delta X
     */
    public double getDeltaX() {
        return deltaX;
    }

    /**
     * Gets the delta Y.
     *
     * @return the delta Y
     */
    public double getDeltaY() {
        return deltaY;
    }

    /**
     * Gets the delta Z.
     *
     * @return the delta Z
     */
    public double getDeltaZ() {
        return deltaZ;
    }

    /**
     * Checks if the entity is on ground.
     *
     * @return true if on ground
     */
    public boolean isOnGround() {
        return onGround;
    }
}
