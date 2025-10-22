package com.poorcraft.server.network;

/**
 * @deprecated Use {@link com.poorcraft.common.network.codec.PacketEncoder}.
 */
@Deprecated
public class PacketEncoder extends com.poorcraft.common.network.codec.PacketEncoder {
    public PacketEncoder(com.poorcraft.common.network.PacketRegistry packetRegistry) {
        super(packetRegistry);
    }
}
