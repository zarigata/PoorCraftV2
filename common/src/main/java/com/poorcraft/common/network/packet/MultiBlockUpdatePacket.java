package com.poorcraft.common.network.packet;

import com.poorcraft.common.network.NetworkUtil;
import com.poorcraft.common.network.Packet;
import io.netty.buffer.ByteBuf;

/**
 * Multi-block update packet for efficient bulk block changes.
 * <p>
 * Contains multiple block updates in a single packet to reduce overhead.
 */
public class MultiBlockUpdatePacket implements Packet {
    private long[] positions;
    private int[] blockIds;

    /**
     * Default constructor for reading from buffer.
     */
    public MultiBlockUpdatePacket() {}

    /**
     * Constructor for creating a multi-block update packet.
     *
     * @param positions array of block positions (packed longs)
     * @param blockIds array of corresponding block IDs
     */
    public MultiBlockUpdatePacket(long[] positions, int[] blockIds) {
        if (positions.length != blockIds.length) {
            throw new IllegalArgumentException("Positions and block IDs arrays must have the same length");
        }
        if (positions.length > 1000) { // Reasonable limit
            throw new IllegalArgumentException("Too many block updates in single packet");
        }
        this.positions = positions.clone();
        this.blockIds = blockIds.clone();
    }

    @Override
    public int getPacketId() {
        return 0x07;
    }

    @Override
    public void write(ByteBuf buf) {
        NetworkUtil.writeVarInt(buf, positions.length);
        for (int i = 0; i < positions.length; i++) {
            NetworkUtil.writeVarInt(buf, (int) (positions[i] >> 38)); // x
            NetworkUtil.writeVarInt(buf, (int) (positions[i] & 0xFFF)); // y
            NetworkUtil.writeVarInt(buf, (int) ((positions[i] >> 12) & 0x3FFFFFF)); // z
            NetworkUtil.writeVarInt(buf, blockIds[i]);
        }
    }

    @Override
    public void read(ByteBuf buf) {
        int count = NetworkUtil.readVarInt(buf);
        if (count < 0 || count > 1000) {
            throw new IllegalArgumentException("Invalid block update count: " + count);
        }

        positions = new long[count];
        blockIds = new int[count];

        for (int i = 0; i < count; i++) {
            int x = NetworkUtil.readVarInt(buf);
            int y = NetworkUtil.readVarInt(buf);
            int z = NetworkUtil.readVarInt(buf);
            positions[i] = ((long) x & 0x3FFFFFF) << 38 | ((long) z & 0x3FFFFFF) << 12 | (y & 0xFFF);
            blockIds[i] = NetworkUtil.readVarInt(buf);
        }
    }

    /**
     * Gets the block positions.
     *
     * @return array of packed positions
     */
    public long[] getPositions() {
        return positions.clone();
    }

    /**
     * Gets the block IDs.
     *
     * @return array of block IDs
     */
    public int[] getBlockIds() {
        return blockIds.clone();
    }

    /**
     * Gets the number of block updates.
     *
     * @return the count
     */
    public int getCount() {
        return positions != null ? positions.length : 0;
    }
}
