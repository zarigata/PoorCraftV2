package com.poorcraft.common.network.packet;

import com.poorcraft.common.network.NetworkUtil;
import com.poorcraft.common.network.Packet;
import io.netty.buffer.ByteBuf;

/**
 * Chunk data packet for sending world chunk information.
 * <p>
 * Contains compressed chunk data including block states, lighting, and biome information.
 */
public class ChunkDataPacket implements Packet {
    private int chunkX;
    private int chunkZ;
    private boolean fullChunk;
    private byte[] chunkData;

    /**
     * Default constructor for reading from buffer.
     */
    public ChunkDataPacket() {}

    /**
     * Constructor for creating a chunk data packet.
     *
     * @param chunkX the chunk X coordinate
     * @param chunkZ the chunk Z coordinate
     * @param fullChunk true if this is a full chunk, false for partial
     * @param chunkData the compressed chunk data
     */
    public ChunkDataPacket(int chunkX, int chunkZ, boolean fullChunk, byte[] chunkData) {
        this.chunkX = chunkX;
        this.chunkZ = chunkZ;
        this.fullChunk = fullChunk;
        this.chunkData = chunkData;
    }

    @Override
    public int getPacketId() {
        return 0x05;
    }

    @Override
    public void write(ByteBuf buf) {
        NetworkUtil.writeVarInt(buf, chunkX);
        NetworkUtil.writeVarInt(buf, chunkZ);
        buf.writeBoolean(fullChunk);
        NetworkUtil.writeByteArray(buf, chunkData);
    }

    @Override
    public void read(ByteBuf buf) {
        chunkX = NetworkUtil.readVarInt(buf);
        chunkZ = NetworkUtil.readVarInt(buf);
        fullChunk = buf.readBoolean();
        chunkData = NetworkUtil.readByteArray(buf);
    }

    /**
     * Gets the chunk X coordinate.
     *
     * @return the chunk X
     */
    public int getChunkX() {
        return chunkX;
    }

    /**
     * Gets the chunk Z coordinate.
     *
     * @return the chunk Z
     */
    public int getChunkZ() {
        return chunkZ;
    }

    /**
     * Checks if this is a full chunk.
     *
     * @return true if full chunk, false otherwise
     */
    public boolean isFullChunk() {
        return fullChunk;
    }

    /**
     * Gets the chunk data.
     *
     * @return the compressed chunk data
     */
    public byte[] getChunkData() {
        return chunkData;
    }
}
