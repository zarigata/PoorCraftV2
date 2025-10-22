package com.poorcraft.common.network.packet;

import com.poorcraft.common.network.NetworkUtil;
import com.poorcraft.common.network.Packet;
import io.netty.buffer.ByteBuf;

import java.util.UUID;

/**
 * Entity spawn packet for creating new entities.
 * <p>
 * Sent when a new entity (player, NPC, etc.) appears in the world.
 */
public class EntitySpawnPacket implements Packet {
    private int entityId;
    private UUID entityUuid;
    private int type;
    private double x;
    private double y;
    private double z;
    private float yaw;
    private float pitch;
    private double velocityX;
    private double velocityY;
    private double velocityZ;

    /**
     * Default constructor for reading from buffer.
     */
    public EntitySpawnPacket() {}

    /**
     * Constructor for creating an entity spawn packet.
     *
     * @param entityId unique entity ID
     * @param entityUuid entity UUID
     * @param type entity type ID
     * @param x position X
     * @param y position Y
     * @param z position Z
     * @param yaw rotation yaw
     * @param pitch rotation pitch
     * @param velocityX velocity X
     * @param velocityY velocity Y
     * @param velocityZ velocity Z
     */
    public EntitySpawnPacket(int entityId, UUID entityUuid, int type, double x, double y, double z,
                           float yaw, float pitch, double velocityX, double velocityY, double velocityZ) {
        this.entityId = entityId;
        this.entityUuid = entityUuid;
        this.type = type;
        this.x = x;
        this.y = y;
        this.z = z;
        this.yaw = yaw;
        this.pitch = pitch;
        this.velocityX = velocityX;
        this.velocityY = velocityY;
        this.velocityZ = velocityZ;
    }

    @Override
    public int getPacketId() {
        return 0x08;
    }

    @Override
    public void write(ByteBuf buf) {
        NetworkUtil.writeVarInt(buf, entityId);
        NetworkUtil.writeUUID(buf, entityUuid);
        NetworkUtil.writeVarInt(buf, type);
        buf.writeDouble(x);
        buf.writeDouble(y);
        buf.writeDouble(z);
        buf.writeByte((byte) (yaw * 256.0f / 360.0f));
        buf.writeByte((byte) (pitch * 256.0f / 360.0f));
        buf.writeShort((short) (velocityX * 8000.0));
        buf.writeShort((short) (velocityY * 8000.0));
        buf.writeShort((short) (velocityZ * 8000.0));
    }

    @Override
    public void read(ByteBuf buf) {
        entityId = NetworkUtil.readVarInt(buf);
        entityUuid = NetworkUtil.readUUID(buf);
        type = NetworkUtil.readVarInt(buf);
        x = buf.readDouble();
        y = buf.readDouble();
        z = buf.readDouble();
        yaw = buf.readByte() * 360.0f / 256.0f;
        pitch = buf.readByte() * 360.0f / 256.0f;
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
     * Gets the entity UUID.
     *
     * @return the entity UUID
     */
    public UUID getEntityUuid() {
        return entityUuid;
    }

    /**
     * Gets the entity type ID.
     *
     * @return the type ID
     */
    public int getType() {
        return type;
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
