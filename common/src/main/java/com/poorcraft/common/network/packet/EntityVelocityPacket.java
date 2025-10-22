package com.poorcraft.common.network.packet;

import com.poorcraft.common.network.NetworkUtil;
import com.poorcraft.common.network.Packet;
import io.netty.buffer.ByteBuf;

/**
 * Entity velocity update packet for velocity changes.
 * <p>
 * Sent to update entity velocities.
 */
public class EntityVelocityPacket implements Packet {
    private int entityId;
    private double velocityX;
    private double velocityY;
    private double velocityZ;

    /**
     * Default constructor for reading from buffer.
     */
    public EntityVelocityPacket() {}

    /**
     * Constructor for creating an entity velocity packet.
     *
     * @param entityId the entity ID
     * @param velocityX velocity X component
     * @param velocityY velocity Y component
     * @param velocityZ velocity Z component
     */
    public EntityVelocityPacket(int entityId, double velocityX, double velocityY, double velocityZ) {
        this.entityId = entityId;
        this.velocityX = velocityX;
        this.velocityY = velocityY;
        this.velocityZ = velocityZ;
    }

    @Override
    public int getPacketId() {
        return 0x0A;
    }

    @Override
    public void write(ByteBuf buf) {
        NetworkUtil.writeVarInt(buf, entityId);
        buf.writeShort((short) (velocityX * 8000.0));
        buf.writeShort((short) (velocityY * 8000.0));
        buf.writeShort((short) (velocityZ * 8000.0));
    }

    @Override
    public void read(ByteBuf buf) {
        entityId = NetworkUtil.readVarInt(buf);
        velocityX = buf.readShort() / 8000.0;
        velocityY = buf.readShort() / 8000.0;
        velocityZ = buf.readShort() / 8000.0;
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
     * Gets the velocity X.
     *
     * @return the velocity X
     */
    public double getVelocityX() {
        return velocityX;
    }

    /**
     * Gets the velocity Y.
     *
     * @return the velocity Y
     */
    public double getVelocityY() {
        return velocityY;
    }

    /**
     * Gets the velocity Z.
     *
     * @return the velocity Z
     */
    public double getVelocityZ() {
        return velocityZ;
    }
}
