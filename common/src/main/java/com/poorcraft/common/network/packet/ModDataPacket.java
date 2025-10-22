package com.poorcraft.common.network.packet;

import com.poorcraft.common.network.NetworkUtil;
import com.poorcraft.common.network.Packet;
import io.netty.buffer.ByteBuf;

import java.util.Arrays;

public class ModDataPacket implements Packet {
    private String modId;
    private int chunkIndex;
    private int totalChunks;
    private byte[] data;
    private String checksum;

    public ModDataPacket() {
    }

    public ModDataPacket(String modId, int chunkIndex, int totalChunks, byte[] data, String checksum) {
        this.modId = modId;
        this.chunkIndex = chunkIndex;
        this.totalChunks = totalChunks;
        this.data = data;
        this.checksum = checksum;
    }

    public String getModId() {
        return modId;
    }

    public int getChunkIndex() {
        return chunkIndex;
    }

    public int getTotalChunks() {
        return totalChunks;
    }

    public byte[] getData() {
        return data;
    }

    public String getChecksum() {
        return checksum;
    }

    @Override
    public int getPacketId() {
        return 0x15;
    }

    @Override
    public void write(ByteBuf buf) {
        NetworkUtil.writeString(buf, modId);
        NetworkUtil.writeVarInt(buf, chunkIndex);
        NetworkUtil.writeVarInt(buf, totalChunks);
        NetworkUtil.writeVarInt(buf, data.length);
        buf.writeBytes(data);
        buf.writeBoolean(checksum != null);
        if (checksum != null) {
            NetworkUtil.writeString(buf, checksum);
        }
    }

    @Override
    public void read(ByteBuf buf) {
        modId = NetworkUtil.readString(buf);
        chunkIndex = NetworkUtil.readVarInt(buf);
        totalChunks = NetworkUtil.readVarInt(buf);
        int length = NetworkUtil.readVarInt(buf);
        data = new byte[length];
        buf.readBytes(data);
        boolean hasChecksum = buf.readBoolean();
        checksum = hasChecksum ? NetworkUtil.readString(buf) : null;
    }

    @Override
    public String toString() {
        return "ModDataPacket{" +
                "modId='" + modId + '\'' +
                ", chunkIndex=" + chunkIndex +
                ", totalChunks=" + totalChunks +
                ", dataLength=" + (data != null ? data.length : 0) +
                ", checksum='" + checksum + '\'' +
                '}';
    }
}
