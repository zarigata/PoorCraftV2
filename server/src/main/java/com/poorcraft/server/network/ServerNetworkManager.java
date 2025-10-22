package com.poorcraft.server.network;

import com.poorcraft.common.config.Configuration;
import com.poorcraft.common.network.ConnectionState;
import com.poorcraft.common.network.Packet;
import com.poorcraft.common.network.PacketHandler;
import com.poorcraft.common.network.PacketRegistry;
import com.poorcraft.server.GameServer;
import com.poorcraft.server.network.handler.BlockInteractionHandler;
import com.poorcraft.server.network.handler.HandshakeHandler;
import com.poorcraft.server.network.handler.KeepAliveHandler;
import com.poorcraft.server.network.handler.LoginHandler;
import com.poorcraft.server.network.handler.ModRequestHandler;
import com.poorcraft.server.network.handler.PlayerMovementHandler;
import io.netty.bootstrap.ServerBootstrap;
import io.netty.channel.Channel;
import io.netty.channel.ChannelFuture;
import io.netty.channel.ChannelInitializer;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.SocketChannel;
import io.netty.channel.socket.nio.NioServerSocketChannel;
import org.slf4j.Logger;

import java.net.InetSocketAddress;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicLong;

/**
 * Manages the server-side networking using Netty.
 * <p>
 * Handles server bootstrap, connection management, packet routing,
 * and provides utilities for broadcasting and handler registration.
 */
public class ServerNetworkManager {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(ServerNetworkManager.class);

    private final Configuration config;
    private final PacketRegistry packetRegistry;
    private final GameServer gameServer;
    private final Map<Class<? extends Packet>, PacketHandler<? extends Packet>> handlers = new ConcurrentHashMap<>();
    private final Map<Channel, ServerConnection> connections = new ConcurrentHashMap<>();
    private final AtomicLong keepAliveCounter = new AtomicLong(0);

    private EventLoopGroup bossGroup;
    private EventLoopGroup workerGroup;
    private Channel serverChannel;

    /**
     * Creates a new server network manager.
     *
     * @param config the server configuration
     * @param packetRegistry the packet registry
     */
    public ServerNetworkManager(Configuration config, PacketRegistry packetRegistry, GameServer gameServer) {
        this.config = config;
        this.packetRegistry = packetRegistry;
        this.gameServer = gameServer;
    }

    /**
     * Initializes the server network manager.
     * <p>
     * Sets up the packet registry and registers default handlers.
     */
    public void init() {
        packetRegistry.registerAllPackets();
        registerDefaultHandlers();
        LOGGER.info("Server network manager initialized with {} packet types",
            packetRegistry.getClientboundCount() + packetRegistry.getServerboundCount());
    }

    /**
     * Starts the server on the configured port.
     *
     * @throws InterruptedException if startup is interrupted
     */
    public void start() throws InterruptedException {
        int port = config.getInt("network.serverPort", 25565);
        int threads = config.getInt("network.serverThreads", 4);

        bossGroup = new NioEventLoopGroup(1);
        workerGroup = new NioEventLoopGroup(threads);

        try {
            ServerBootstrap bootstrap = new ServerBootstrap();
            bootstrap.group(bossGroup, workerGroup)
                .channel(NioServerSocketChannel.class)
                .childHandler(new ServerChannelInitializer(this, config, packetRegistry));

            ChannelFuture future = bootstrap.bind(port).sync();
            serverChannel = future.channel();

            LOGGER.info("Server started on port {}", port);
        } catch (InterruptedException e) {
            LOGGER.error("Failed to start server", e);
            throw e;
        }
    }

    /**
     * Stops the server and closes all connections.
     */
    public void stop() {
        LOGGER.info("Stopping server network manager...");

        if (serverChannel != null) {
            serverChannel.close();
        }

        if (workerGroup != null) {
            workerGroup.shutdownGracefully();
        }

        if (bossGroup != null) {
            bossGroup.shutdownGracefully();
        }

        // Close all connections
        for (ServerConnection connection : connections.values()) {
            connection.disconnect("Server shutting down");
        }
        connections.clear();

        LOGGER.info("Server network manager stopped");
    }

    /**
     * Registers a packet handler for the given packet type.
     *
     * @param packetClass the packet class
     * @param handler the handler
     * @param <T> the packet type
     */
    public <T extends Packet> void registerHandler(Class<T> packetClass, PacketHandler<T> handler) {
        handlers.put(packetClass, handler);
        LOGGER.debug("Registered handler for {}", packetClass.getSimpleName());
    }

    /**
     * Dispatches a packet to the appropriate handler.
     *
     * @param packet the packet to handle
     * @param connection the connection context
     */
    @SuppressWarnings("unchecked")
    public void handlePacket(Packet packet, ServerConnection connection) {
        PacketHandler<Packet> handler = (PacketHandler<Packet>) handlers.get(packet.getClass());
        if (handler != null) {
            handler.handle(packet, connection);
        } else {
            LOGGER.warn("No handler registered for packet type: {}", packet.getClass().getSimpleName());
        }
    }

    /**
     * Broadcasts a packet to all connected clients.
     *
     * @param packet the packet to broadcast
     */
    public void broadcast(Packet packet) {
        for (ServerConnection connection : connections.values()) {
            if (connection.getConnectionState() == ConnectionState.PLAY) {
                connection.sendPacket(packet);
            }
        }
    }

    /**
     * Gets the server configuration.
     *
     * @return the configuration
     */
    public Configuration getConfig() {
        return config;
    }

    /**
     * Generates a new keep-alive ID.
     *
     * @return the keep-alive ID
     */
    public long nextKeepAliveId() {
        return keepAliveCounter.incrementAndGet();
    }

    /**
     * Called when a new connection is established.
     *
     * @param channel the channel
     * @param connection the connection wrapper
     */
    public void onConnectionEstablished(Channel channel, ServerConnection connection) {
        connections.put(channel, connection);
        LOGGER.info("New connection from {}", channel.remoteAddress());
    }

    /**
     * Called when a connection is closed.
     *
     * @param channel the channel
     */
    public void onConnectionClosed(Channel channel) {
        ServerConnection connection = connections.remove(channel);
        if (connection != null) {
            LOGGER.info("Connection closed: {}", channel.remoteAddress());
        }
    }

    /**
     * Performs periodic maintenance tasks.
     * <p>
     * Should be called regularly (e.g., 20 TPS) to handle timeouts and keep-alives.
     */
    public void tick() {
        long now = System.currentTimeMillis();
        long timeout = config.getLong("network.connectionTimeout", 30000L);

        for (ServerConnection connection : connections.values()) {
            if (now - connection.getLastPacketTime() > timeout) {
                connection.disconnect("Connection timed out");
            } else if (connection.shouldSendKeepAlive()) {
                connection.sendKeepAlive();
            }
        }
    }

    /**
     * Registers default packet handlers.
     */
    private void registerDefaultHandlers() {
        registerHandler(com.poorcraft.common.network.packet.HandshakePacket.class, new HandshakeHandler());
        registerHandler(com.poorcraft.common.network.packet.LoginStartPacket.class, new LoginHandler(gameServer));
        PlayerMovementHandler movementHandler = new PlayerMovementHandler(gameServer);
        registerHandler(com.poorcraft.common.network.packet.PlayerPositionPacket.class, movementHandler);
        registerHandler(com.poorcraft.common.network.packet.PlayerLookPacket.class, movementHandler);
        registerHandler(com.poorcraft.common.network.packet.PlayerPositionLookPacket.class, movementHandler);
        BlockInteractionHandler blockHandler = new BlockInteractionHandler(gameServer);
        registerHandler(com.poorcraft.common.network.packet.PlayerDiggingPacket.class, blockHandler);
        registerHandler(com.poorcraft.common.network.packet.PlayerBlockPlacementPacket.class, blockHandler);
        registerHandler(com.poorcraft.common.network.packet.KeepAlivePacket.class, new KeepAliveHandler());
        registerHandler(com.poorcraft.common.network.packet.ModRequestPacket.class, new ModRequestHandler(gameServer));
    }

    public GameServer getGameServer() {
        return gameServer;
    }
}
