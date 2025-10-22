package com.poorcraft.server;

import com.poorcraft.common.Constants;
import com.poorcraft.common.config.Configuration;
import org.slf4j.Logger;

import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

/**
 * Main entry point for the PoorCraft dedicated server.
 * 
 * <p>This is a placeholder implementation for Phase 1. Phase 5 will add:
 * <ul>
 *   <li>Netty server initialization</li>
 *   <li>World loading and management</li>
 *   <li>Player connection handling</li>
 *   <li>Server console with JLine</li>
 *   <li>Server tick loop</li>
 * </ul>
 */
public class Main {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(Main.class);
    
    public static void main(String[] args) {
        LOGGER.info("=== {} Server v{} ===", Constants.Game.NAME, Constants.Game.VERSION);
        LOGGER.info("Java Version: {}", System.getProperty("java.version"));
        LOGGER.info("OS: {} {} ({})",
            System.getProperty("os.name"),
            System.getProperty("os.version"),
            System.getProperty("os.arch")
        );

        // Parse command line arguments
        ServerConfig serverConfig = parseArguments(args);

        // Add shutdown hook for graceful cleanup
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            LOGGER.info("Shutting down server...");
            // GameServer will handle cleanup
        }, "Shutdown-Hook"));

        try {
            // Ensure config directory exists and copy default config if needed
            Path configDir = Paths.get("config");
            Path serverConfigPath = configDir.resolve("server.yml");

            if (!Files.exists(configDir)) {
                Files.createDirectories(configDir);
            }

            if (!Files.exists(serverConfigPath)) {
                LOGGER.info("Server configuration not found, creating default config at {}", serverConfigPath);
                try (InputStream defaultConfig = Main.class.getResourceAsStream("/server-config.yml")) {
                    if (defaultConfig != null) {
                        Files.copy(defaultConfig, serverConfigPath);
                        LOGGER.info("Default server configuration copied to {}. Edit this file to customize settings.", serverConfigPath);
                    } else {
                        LOGGER.warn("Default server-config.yml resource not found in classpath");
                    }
                } catch (IOException e) {
                    LOGGER.warn("Could not copy default server configuration", e);
                }
            }

            // Load server configuration
            Configuration config;
            try {
                config = new Configuration(serverConfigPath);
            } catch (IOException e) {
                LOGGER.warn("Could not load server configuration, using defaults", e);
                config = new Configuration();
            }

            // Get server port from config or command line
            int port = serverConfig.port != -1 ?
                serverConfig.port :
                config.getInt("network.serverPort", Constants.Network.DEFAULT_SERVER_PORT);

            LOGGER.info("Server starting on port {}...", port);

            // Create and initialize game server
            GameServer gameServer = new GameServer(config);
            gameServer.init();

            // Start game server (this will run the tick loop)
            gameServer.start();

            LOGGER.info("Server initialization complete");
            LOGGER.info("Press Ctrl+C to stop");

            // Keep main thread alive while server is running
            while (gameServer.isRunning()) {
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                    break;
                }
            }

            // Stop game server
            gameServer.stop();

            LOGGER.info("Server stopped");

        } catch (Exception e) {
            LOGGER.error("Fatal error in server", e);
            System.exit(1);
        }
    }
    
    /**
     * Parses command line arguments.
     * 
     * @param args Command line arguments
     * @return Parsed server configuration
     */
    private static ServerConfig parseArguments(String[] args) {
        ServerConfig config = new ServerConfig();
        
        for (int i = 0; i < args.length; i++) {
            switch (args[i]) {
                case "--port":
                case "-p":
                    if (i + 1 < args.length) {
                        try {
                            config.port = Integer.parseInt(args[++i]);
                        } catch (NumberFormatException e) {
                            LOGGER.warn("Invalid port number: {}", args[i]);
                        }
                    }
                    break;
                    
                case "--world":
                case "-w":
                    if (i + 1 < args.length) {
                        config.worldName = args[++i];
                    }
                    break;
                    
                case "--help":
                case "-h":
                    printHelp();
                    System.exit(0);
                    break;
            }
        }
        
        return config;
    }
    
    /**
     * Prints command line help.
     */
    private static void printHelp() {
        System.out.println("PoorCraft Server v" + Constants.Game.VERSION);
        System.out.println();
        System.out.println("Usage: java -jar poorcraft-server.jar [options]");
        System.out.println();
        System.out.println("Options:");
        System.out.println("  -p, --port <port>     Server port (default: 25565)");
        System.out.println("  -w, --world <name>    World name (default: world)");
        System.out.println("  -h, --help            Show this help message");
    }
    
    /**
     * Server configuration from command line arguments.
     */
    private static class ServerConfig {
        int port = -1;
        String worldName = "world";
    }
}
