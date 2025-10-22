package com.poorcraft.common.network.codec;

import com.poorcraft.common.network.Packet;
import com.poorcraft.common.network.PacketRegistry;
import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.ByteToMessageDecoder;
import org.slf4j.Logger;

import java.util.List;

/**
 * Decodes incoming bytes into packets.
 * <p>
 * Reads packet ID, instantiates the appropriate packet class,
 * and calls read() to populate the packet data.
 */
public class PacketDecoder extends ByteToMessageDecoder {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(PacketDecoder.class);

    private final PacketRegistry packetRegistry;
    private final boolean clientbound;

    /**
     * Creates a new packet decoder.
     *
     * @param packetRegistry the packet registry
     * @param clientbound true if packets are clientbound (server → client), false if serverbound
     */
    public PacketDecoder(PacketRegistry packetRegistry, boolean clientbound) {
        this.packetRegistry = packetRegistry;
        this.clientbound = clientbound;
    }

    @Override
    protected void decode(ChannelHandlerContext ctx, ByteBuf in, List<Object> out) {
        if (!in.isReadable()) {
            return;
        }

        // Read packet ID as varint
        int packetId;
        try {
            packetId = com.poorcraft.common.network.NetworkUtil.readVarInt(in);
        } catch (Exception e) {
            LOGGER.warn("Failed to read packet ID from {}", ctx.channel().remoteAddress(), e);
            ctx.close();
            return;
        }

        // Instantiate packet
        Packet packet = packetRegistry.createPacket(packetId, clientbound);
        if (packet == null) {
            LOGGER.warn("Unknown packet ID: {} from {}", packetId, ctx.channel().remoteAddress());
            ctx.close();
            return;
        }

        try {
            // Read packet data
            packet.read(in);

            // Validate buffer is fully consumed
            if (in.isReadable()) {
                LOGGER.warn("Packet {} did not consume all data from {}", packet.getClass().getSimpleName(), ctx.channel().remoteAddress());
                ctx.close();
                return;
            }

            out.add(packet);

        } catch (Exception e) {
            LOGGER.error("Failed to decode packet {} from {}", packet.getClass().getSimpleName(), ctx.channel().remoteAddress(), e);
            ctx.close();
        }
    }
}
