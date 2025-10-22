package com.poorcraft.common.network.packet;

import com.poorcraft.common.network.NetworkUtil;
import com.poorcraft.common.network.Packet;
import io.netty.buffer.ByteBuf;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

public class ModListPacket implements Packet {
    public static class ModInfo {
        private final String id;
        private final String version;
        private final long size;
        private final String checksum;

        public ModInfo(String id, String version, long size, String checksum) {
            this.id = Objects.requireNonNull(id, "id");
            this.version = Objects.requireNonNull(version, "version");
            this.size = size;
            this.checksum = Objects.requireNonNull(checksum, "checksum");
        }

        public String getId() {
            return id;
        }

        public String getVersion() {
            return version;
        }

        public long getSize() {
            return size;
        }

        public String getChecksum() {
            return checksum;
        }
    }

    private final List<ModInfo> mods = new ArrayList<>();

    public ModListPacket() {
    }

    public ModListPacket(List<ModInfo> mods) {
        this.mods.addAll(mods);
    }

    public List<ModInfo> getMods() {
        return mods;
    }

    @Override
    public int getPacketId() {
        return 0x13;
    }

    @Override
    public void write(ByteBuf buf) {
        NetworkUtil.writeVarInt(buf, mods.size());
        for (ModInfo mod : mods) {
            NetworkUtil.writeString(buf, mod.getId());
            NetworkUtil.writeString(buf, mod.getVersion());
            buf.writeLong(mod.getSize());
            NetworkUtil.writeString(buf, mod.getChecksum());
        }
    }

    @Override
    public void read(ByteBuf buf) {
        mods.clear();
        int count = NetworkUtil.readVarInt(buf);
        for (int i = 0; i < count; i++) {
            String id = NetworkUtil.readString(buf);
            String version = NetworkUtil.readString(buf);
            long size = buf.readLong();
            String checksum = NetworkUtil.readString(buf);
            mods.add(new ModInfo(id, version, size, checksum));
        }
    }
}
