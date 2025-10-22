package com.poorcraft.server;

import com.poorcraft.api.ModAPI;
import com.poorcraft.common.config.Configuration;
import com.poorcraft.common.event.EventBus;
import com.poorcraft.common.network.PacketRegistry;
import com.poorcraft.common.network.NetworkServer;
import com.poorcraft.common.network.packet.LoginSuccessPacket;
import com.poorcraft.common.registry.RegistryManager;
import com.poorcraft.common.registry.Registries;
import com.poorcraft.common.util.Logger;
import com.poorcraft.server.entity.ServerEntityManager;
import com.poorcraft.server.network.ServerNetworkManager;
import com.poorcraft.server.network.session.PlayerSessionManager;
import com.poorcraft.server.world.ServerWorldManager;
import com.poorcraft.server.mod.ServerModManager;
import org.slf4j.Logger;

import java.util.Map;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Main server coordinator for the PoorCraft dedicated server.
 * <p>
 * Manages networking, world updates, player sessions, and runs the 20 TPS game loop.
 */
public class GameServer {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(GameServer.class);

    private final Configuration config;
    private final ServerEntityManager entityManager;
    private final PacketRegistry packetRegistry;
    private final ServerNetworkManager networkManager;
    private final PlayerSessionManager sessionManager;
    private final ServerWorldManager worldManager;
    private final EventBus eventBus;
    private final RegistryManager registryManager;
    private final ServerModManager modManager;
    private final Map<UUID, PlayerSession> playerSessions = new ConcurrentHashMap<>();

    private ScheduledExecutorService tickExecutor;
    private volatile boolean running = false;

    /**
     * Creates a new game server.
     *
     * @param config the server configuration
     */
    public GameServer(Configuration config) {
        this.config = config;
        this.entityManager = new ServerEntityManager(this);
        this.packetRegistry = new PacketRegistry();
        this.networkManager = new ServerNetworkManager(config, packetRegistry, this);
        this.sessionManager = new PlayerSessionManager(this);
        this.worldManager = new ServerWorldManager(this);
        this.eventBus = new EventBus();
        this.registryManager = new RegistryManager();
        ModAPI.initialize(eventBus, registryManager);
        Registries.init();
        this.modManager = new ServerModManager(config, eventBus, registryManager);
        this.worldManager.loadWorlds();
    }

    /**
     * Initializes the game server.
     * <p>
     * Sets up networking, registers handlers, and prepares the world.
     */
    public void init() {
        LOGGER.info("Initializing PoorCraft Server...");
        try {
            worldManager.loadWorlds();
            networkManager.init();
            modManager.init();
            registryManager.freezeAll();
        } catch (Exception e) {
            LOGGER.error("Failed to initialize server", e);
            stop();
        }
    }

    /**
     * Starts the game server.
     * <p>
     * Begins the tick loop and starts accepting connections.
     *
     * @throws InterruptedException if startup is interrupted
     */
    public void start() throws InterruptedException {
        LOGGER.info("Starting PoorCraft Server...");

        // Start networking
        networkManager.start();

        // Start tick loop (20 TPS = 50ms per tick)
        running = true;
        tickExecutor = Executors.newSingleThreadScheduledExecutor(r -> {
            Thread thread = new Thread(r, "Server-Tick");
            thread.setDaemon(true);
            return thread;
        });

        tickExecutor.scheduleAtFixedRate(this::tick, 0, 50, TimeUnit.MILLISECONDS);

        LOGGER.info("PoorCraft Server started successfully");
    }

    /**
     * Stops the game server.
     * <p>
     * Shuts down networking and saves world state.
     */
    public void stop() {
        LOGGER.info("Stopping PoorCraft Server...");
        running = false;
        try {
            networkManager.stop();
            modManager.shutdown();
            entityManager.shutdown();
            worldManager.saveAll();
        } catch (Exception e) {
            LOGGER.error("Error while stopping server", e);
        }
        if (tickExecutor != null) {
            tickExecutor.shutdown();
            try {
                if (!tickExecutor.awaitTermination(5, TimeUnit.SECONDS)) {
                    tickExecutor.shutdownNow();
                }
            } catch (InterruptedException e) {
                tickExecutor.shutdownNow();
                Thread.currentThread().interrupt();
            }
        }
        LOGGER.info("PoorCraft Server stopped");
    }

    /**
     * Main game tick loop.
     * <p>
     * Updates world state, processes player actions, and handles networking.
     */
    private void tick() {
        if (!running) {
            return;
        }

        try {
            long startTime = System.currentTimeMillis();

            // Update networking (timeouts, keep-alives)
            networkManager.tick();

            // Update world
            worldManager.tick();

            // Update player sessions
            for (PlayerSession session : playerSessions.values()) {
                session.tick();
            }

            long tickTime = System.currentTimeMillis() - startTime;

            // Log if tick takes too long (>25ms for 20 TPS)
            if (tickTime > 25) {
                LOGGER.warn("Slow tick detected: {}ms", tickTime);
            }

        } catch (Exception e) {
            LOGGER.error("Error in game tick", e);
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
     * Gets the network manager.
     *
     * @return the network manager
     */
    public ServerNetworkManager getNetworkManager() {
        return networkManager;
    }

    public PacketRegistry getPacketRegistry() {
        return packetRegistry;
    }

    /**
     * Gets the server world.
     *
     * @return the world
     */
    public ServerWorld getWorld() {
        return world;
    }

    public PlayerSession getPlayerSession(UUID uuid) {
        return playerSessions.get(uuid);
    }

    public void handlePlayerJoin(ServerConnection connection, UUID uuid, String username) {
        PlayerSession session = world.createPlayerSession(connection, uuid, username);
        playerSessions.put(uuid, session);
        connection.setPlayerSession(session);
        world.spawnPlayerEntity(session);
        LOGGER.info("Player {} joined. Active sessions: {}", username, playerSessions.size());
    }

    public void handlePlayerLeave(UUID uuid) {
        PlayerSession session = playerSessions.remove(uuid);
        if (session != null) {
            world.despawnPlayerEntity(session);
            world.removePlayerSession(uuid);
            session.disconnect();
            LOGGER.info("Player {} disconnected. Active sessions: {}", uuid, playerSessions.size());
        }
    }

    /**
     * Checks if the server is running.
     *
     * @return true if running, false otherwise
     */
    public boolean isRunning() {
        return running;
    }

    public ServerModManager getModManager() {
        return modManager;
    }

    public EventBus getEventBus() {
        return eventBus;
    }
}
