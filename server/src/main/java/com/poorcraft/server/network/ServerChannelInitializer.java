package com.poorcraft.server.network;

import com.poorcraft.common.config.Configuration;
import com.poorcraft.common.network.PacketRegistry;
import com.poorcraft.common.network.codec.PacketDecoder;
import com.poorcraft.common.network.codec.PacketEncoder;
import io.netty.channel.ChannelInitializer;
import io.netty.channel.socket.SocketChannel;
import io.netty.handler.codec.LengthFieldBasedFrameDecoder;
import io.netty.handler.codec.LengthFieldPrepender;

/**
 * Initializes server-side channels with the appropriate handlers.
 * <p>
 * Sets up the Netty pipeline for server connections including framing,
 * compression (if enabled), and packet encoding/decoding.
 */
public class ServerChannelInitializer extends ChannelInitializer<SocketChannel> {
    private final ServerNetworkManager networkManager;
    private final Configuration config;
    private final PacketRegistry packetRegistry;

    /**
     * Creates a new server channel initializer.
     *
     * @param networkManager the server network manager
     * @param config the server configuration
     * @param packetRegistry the packet registry
     */
    public ServerChannelInitializer(ServerNetworkManager networkManager, Configuration config, PacketRegistry packetRegistry) {
        this.networkManager = networkManager;
        this.config = config;
        this.packetRegistry = packetRegistry;
    }

    @Override
    protected void initChannel(SocketChannel ch) {
        // Frame decoder for length-prefixed messages (4-byte length)
        ch.pipeline().addLast("frameDecoder",
            new LengthFieldBasedFrameDecoder(
                config.getInt("network.maxPacketSize", 2097152),
                0, 4, 0, 4));

        // Frame prepender for length-prefixed messages
        ch.pipeline().addLast("framePrepender", new LengthFieldPrepender(4));

        // Compression (if enabled)
        if (config.getBoolean("network.enableCompression", true)) {
            int threshold = config.getInt("network.compressionThreshold", 256);
            ch.pipeline().addLast("decompressor",
                new io.netty.handler.codec.compression.ZlibDecoder());
            ch.pipeline().addLast("compressor",
                new io.netty.handler.codec.compression.ZlibEncoder(threshold));
        }

        // Packet decoder and encoder
        ch.pipeline().addLast("packetDecoder", new PacketDecoder(packetRegistry, false));
        ch.pipeline().addLast("packetEncoder", new PacketEncoder(packetRegistry));

        // Main packet handler
        ch.pipeline().addLast("packetHandler", new ServerPacketHandler(networkManager));
    }
}
