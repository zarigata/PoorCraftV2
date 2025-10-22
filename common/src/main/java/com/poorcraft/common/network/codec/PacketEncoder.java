package com.poorcraft.common.network.codec;

import com.poorcraft.common.network.Packet;
import com.poorcraft.common.network.PacketRegistry;
import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.MessageToByteEncoder;
import org.slf4j.Logger;

/**
 * Encodes packets into bytes for transmission.
 * <p>
 * Writes packet ID and packet data to the buffer.
 */
public class PacketEncoder extends MessageToByteEncoder<Packet> {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(PacketEncoder.class);

    private final PacketRegistry packetRegistry;

    /**
     * Creates a new packet encoder.
     *
     * @param packetRegistry the packet registry
     */
    public PacketEncoder(PacketRegistry packetRegistry) {
        this.packetRegistry = packetRegistry;
    }

    @Override
    protected void encode(ChannelHandlerContext ctx, Packet packet, ByteBuf out) {
        int packetId = packetRegistry.getPacketId(packet.getClass());
        if (packetId == -1) {
            LOGGER.error("Unknown packet type: {} for {}", packet.getClass().getSimpleName(), ctx.channel().remoteAddress());
            ctx.close();
            return;
        }

        try {
            // Write packet ID
            com.poorcraft.common.network.NetworkUtil.writeVarInt(out, packetId);

            // Write packet data
            packet.write(out);

        } catch (Exception e) {
            LOGGER.error("Failed to encode packet {} for {}", packet.getClass().getSimpleName(), ctx.channel().remoteAddress(), e);
            ctx.close();
        }
    }
}
