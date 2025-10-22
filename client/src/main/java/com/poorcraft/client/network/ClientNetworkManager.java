package com.poorcraft.client.network;

import com.poorcraft.common.config.Configuration;
import com.poorcraft.common.network.ConnectionState;
import com.poorcraft.common.network.Packet;
import com.poorcraft.common.network.PacketHandler;
import com.poorcraft.common.network.PacketRegistry;
import com.poorcraft.common.network.packet.HandshakePacket;
import com.poorcraft.common.network.packet.LoginStartPacket;
import io.netty.bootstrap.Bootstrap;
import io.netty.channel.Channel;
import io.netty.channel.ChannelFuture;
import io.netty.channel.ChannelInitializer;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.SocketChannel;
import io.netty.channel.socket.nio.NioSocketChannel;
import org.slf4j.Logger;

import java.net.InetSocketAddress;
import java.util.Map;
import java.util.Queue;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;

/**
 * Manages the client-side networking using Netty.
 * <p>
 * Handles connection to server, packet sending/receiving, and handler registration.
 */
public class ClientNetworkManager {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(ClientNetworkManager.class);

    private final Configuration config;
    private final PacketRegistry packetRegistry;
    private final Map<Class<? extends Packet>, PacketHandler<? extends Packet>> handlers = new ConcurrentHashMap<>();

    private EventLoopGroup workerGroup;
    private Channel channel;
    private ClientConnection connection;
    private final Queue<Runnable> mainThreadQueue = new ConcurrentLinkedQueue<>();

    /**
     * Creates a new client network manager.
     *
     * @param config the client configuration
     */
    public ClientNetworkManager(Configuration config) {
        this.config = config;
        this.packetRegistry = new PacketRegistry();
    }

    /**
     * Initializes the client network manager.
     * <p>
     * Sets up the packet registry and registers default handlers.
     */
    public void init() {
        packetRegistry.registerAllPackets();
        LOGGER.info("Client network manager initialized with {} packet types",
            packetRegistry.getClientboundCount() + packetRegistry.getServerboundCount());
    }

    /**
     * Connects to a server.
     *
     * @param host the server host
     * @param port the server port
     * @throws InterruptedException if connection is interrupted
     */
    public void connect(String host, int port) throws InterruptedException {
        if (channel != null && channel.isActive()) {
            LOGGER.warn("Already connected to server");
            return;
        }

        int threads = config.getInt("network.clientThreads", 2);
        workerGroup = new NioEventLoopGroup(threads);

        try {
            Bootstrap bootstrap = new Bootstrap();
            bootstrap.group(workerGroup)
                .channel(NioSocketChannel.class)
                .handler(new ClientChannelInitializer(this, config, packetRegistry));

            ChannelFuture future = bootstrap.connect(host, port).sync();
            channel = future.channel();

            // Create connection wrapper
            connection = new ClientConnection(channel, this);

            LOGGER.info("Connected to server {}:{}", host, port);

            // Send handshake
            sendHandshake();

        } catch (InterruptedException e) {
            LOGGER.error("Failed to connect to server", e);
            disconnect();
            throw e;
        }
    }

    /**
     * Disconnects from the server.
     */
    public void disconnect() {
        LOGGER.info("Disconnecting from server...");

        if (channel != null) {
            channel.close();
            channel = null;
        }

        if (workerGroup != null) {
            workerGroup.shutdownGracefully();
            workerGroup = null;
        }

        connection = null;

        LOGGER.info("Disconnected from server");
    }

    /**
     * Sends a handshake packet to the server.
     */
    private void sendHandshake() {
        HandshakePacket handshake = new HandshakePacket(
            com.poorcraft.common.Constants.Game.PROTOCOL_VERSION,
            "localhost", // Will be updated by caller
            25565, // Will be updated by caller
            1 // LOGIN state
        );

        connection.sendPacket(handshake);
        LOGGER.debug("Sent handshake packet");
    }

    /**
     * Sends a login start packet.
     *
     * @param username the player username
     */
    public void sendLoginStart(String username) {
        if (connection == null || !connection.isConnected()) {
            LOGGER.error("Not connected to server");
            return;
        }

        LoginStartPacket loginStart = new LoginStartPacket(username);
        connection.sendPacket(loginStart);
        LOGGER.debug("Sent login start packet for user: {}", username);
    }

    /**
     * Sends a packet to the server.
     *
     * @param packet the packet to send
     */
    public void sendPacket(Packet packet) {
        if (connection != null) {
            connection.sendPacket(packet);
        }
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
     * Enqueues work to be executed on the main game thread.
     *
     * @param task the task to run during {@link #tick()}
     */
    public void enqueue(Runnable task) {
        if (task != null) {
            mainThreadQueue.offer(task);
        }
    }

    /**
     * Dispatches a packet to the appropriate handler.
     *
     * @param packet the packet to handle
     * @param connection the connection context
     */
    @SuppressWarnings("unchecked")
    public void handlePacket(Packet packet, ClientConnection connection) {
        PacketHandler<Packet> handler = (PacketHandler<Packet>) handlers.get(packet.getClass());
        if (handler != null) {
            handler.handle(packet, connection);
        } else {
            LOGGER.warn("No handler registered for packet type: {}", packet.getClass().getSimpleName());
        }
    }

    /**
     * Performs periodic maintenance tasks.
     * <p>
     * Should be called regularly from the main game loop.
     */
    public void tick() {
        if (connection != null) {
            connection.tick();
        }

        Runnable task;
        while ((task = mainThreadQueue.poll()) != null) {
            try {
                task.run();
            } catch (Exception e) {
                LOGGER.error("Error executing queued network task", e);
            }
        }
    }

    /**
     * Gets the current connection state.
     *
     * @return the connection state, or DISCONNECTED if not connected
     */
    public ConnectionState getConnectionState() {
        return connection != null ? connection.getConnectionState() : ConnectionState.DISCONNECTED;
    }

    /**
     * Checks if connected to a server.
     *
     * @return true if connected, false otherwise
     */
    public boolean isConnected() {
        return connection != null && connection.isConnected();
    }

    /**
     * Gets the client configuration.
     *
     * @return the configuration
     */
    public Configuration getConfig() {
        return config;
    }

    /**
     * Gets the packet registry.
     *
     * @return the packet registry
     */
    public PacketRegistry getPacketRegistry() {
        return packetRegistry;
    }
}
