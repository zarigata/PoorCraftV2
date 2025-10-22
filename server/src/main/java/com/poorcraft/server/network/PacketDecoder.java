package com.poorcraft.server.network;

/**
 * @deprecated Use {@link com.poorcraft.common.network.codec.PacketDecoder}.
 */
@Deprecated
public class PacketDecoder extends com.poorcraft.common.network.codec.PacketDecoder {
    public PacketDecoder(com.poorcraft.common.network.PacketRegistry packetRegistry) {
        super(packetRegistry, false);
    }
}
