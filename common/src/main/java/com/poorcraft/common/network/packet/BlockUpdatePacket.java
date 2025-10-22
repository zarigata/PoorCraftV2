package com.poorcraft.common.network.packet;

import com.poorcraft.common.network.NetworkUtil;
import com.poorcraft.common.network.Packet;
import io.netty.buffer.ByteBuf;

/**
 * Block update packet for single block changes.
 * <p>
 * Notifies clients of block state changes at a specific position.
 */
public class BlockUpdatePacket implements Packet {
    private long position;
    private int blockId;

    /**
     * Default constructor for reading from buffer.
     */
    public BlockUpdatePacket() {}

    /**
     * Constructor for creating a block update packet.
     *
     * @param x the block X coordinate
     * @param y the block Y coordinate
     * @param z the block Z coordinate
     * @param blockId the new block ID
     */
    public BlockUpdatePacket(int x, int y, int z, int blockId) {
        this.position = ((long) x & 0x3FFFFFF) << 38 | ((long) z & 0x3FFFFFF) << 12 | (y & 0xFFF);
        this.blockId = blockId;
    }

    @Override
    public int getPacketId() {
        return 0x06;
    }

    @Override
    public void write(ByteBuf buf) {
        NetworkUtil.writeVarInt(buf, (int) (position >> 38)); // x
        NetworkUtil.writeVarInt(buf, (int) (position & 0xFFF)); // y
        NetworkUtil.writeVarInt(buf, (int) ((position >> 12) & 0x3FFFFFF)); // z
        NetworkUtil.writeVarInt(buf, blockId);
    }

    @Override
    public void read(ByteBuf buf) {
        int x = NetworkUtil.readVarInt(buf);
        int y = NetworkUtil.readVarInt(buf);
        int z = NetworkUtil.readVarInt(buf);
        position = ((long) x & 0x3FFFFFF) << 38 | ((long) z & 0x3FFFFFF) << 12 | (y & 0xFFF);
        blockId = NetworkUtil.readVarInt(buf);
    }

    /**
     * Gets the block position as a packed long.
     *
     * @return the packed position
     */
    public long getPosition() {
        return position;
    }

    /**
     * Gets the block X coordinate.
     *
     * @return the X coordinate
     */
    public int getX() {
        return (int) (position >> 38);
    }

    /**
     * Gets the block Y coordinate.
     *
     * @return the Y coordinate
     */
    public int getY() {
        return (int) (position & 0xFFF);
    }

    /**
     * Gets the block Z coordinate.
     *
     * @return the Z coordinate
     */
    public int getZ() {
        return (int) ((position >> 12) & 0x3FFFFFF);
    }

    /**
     * Gets the new block ID.
     *
     * @return the block ID
     */
    public int getBlockId() {
        return blockId;
    }
}
