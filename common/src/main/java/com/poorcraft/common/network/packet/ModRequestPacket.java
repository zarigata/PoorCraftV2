package com.poorcraft.common.network.packet;

import com.poorcraft.common.network.NetworkUtil;
import com.poorcraft.common.network.Packet;
import io.netty.buffer.ByteBuf;

public class ModRequestPacket implements Packet {
    private String modId;

    public ModRequestPacket() {
    }

    public ModRequestPacket(String modId) {
        this.modId = modId;
    }

    public String getModId() {
        return modId;
    }

    @Override
    public int getPacketId() {
        return 0x14;
    }

    @Override
    public void write(ByteBuf buf) {
        NetworkUtil.writeString(buf, modId);
    }

    @Override
    public void read(ByteBuf buf) {
        modId = NetworkUtil.readString(buf);
    }
}
