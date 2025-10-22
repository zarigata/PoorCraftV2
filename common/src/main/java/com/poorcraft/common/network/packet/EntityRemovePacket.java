package com.poorcraft.common.network.packet;

import com.poorcraft.common.network.NetworkUtil;
import com.poorcraft.common.network.Packet;
import io.netty.buffer.ByteBuf;

/**
 * Entity remove packet for removing entities.
 * <p>
 * Sent when an entity is destroyed or despawned.
 */
public class EntityRemovePacket implements Packet {
    private int[] entityIds;

    /**
     * Default constructor for reading from buffer.
     */
    public EntityRemovePacket() {}

    /**
     * Constructor for creating an entity remove packet.
     *
     * @param entityIds array of entity IDs to remove
     */
    public EntityRemovePacket(int[] entityIds) {
        this.entityIds = entityIds.clone();
    }

    @Override
    public int getPacketId() {
        return 0x0B;
    }

    @Override
    public void write(ByteBuf buf) {
        NetworkUtil.writeVarInt(buf, entityIds.length);
        for (int entityId : entityIds) {
            NetworkUtil.writeVarInt(buf, entityId);
        }
    }

    @Override
    public void read(ByteBuf buf) {
        int count = NetworkUtil.readVarInt(buf);
        entityIds = new int[count];
        for (int i = 0; i < count; i++) {
            entityIds[i] = NetworkUtil.readVarInt(buf);
        }
    }

    /**
     * Gets the entity IDs to remove.
     *
     * @return array of entity IDs
     */
    public int[] getEntityIds() {
        return entityIds.clone();
    }
}
