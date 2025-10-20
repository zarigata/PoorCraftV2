package com.poorcraft.client;

import com.poorcraft.common.Constants;
import com.poorcraft.common.config.Configuration;
import com.poorcraft.common.util.Logger;
import com.poorcraft.core.Engine;
import org.slf4j.Logger;

import java.io.IOException;
import java.nio.file.Paths;

/**
 * Main entry point for the PoorCraft client application.
 * 
 * <p>This is a placeholder implementation for Phase 1. Phase 2 will add:
 * <ul>
 *   <li>GLFW window creation</li>
 *   <li>OpenGL context initialization</li>
 *   <li>Integration with Engine game loop</li>
 *   <li>Input handling setup</li>
 * </ul>
 */
public class Main {
    private static final Logger LOGGER = Logger.getLogger(Main.class);
    
    public static void main(String[] args) {
        LOGGER.info("=== {} v{} ===", Constants.Game.NAME, Constants.Game.VERSION);
        LOGGER.info("Java Version: {}", System.getProperty("java.version"));
        LOGGER.info("OS: {} {} ({})", 
            System.getProperty("os.name"),
            System.getProperty("os.version"),
            System.getProperty("os.arch")
        );
        
        // Add shutdown hook for graceful cleanup
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            LOGGER.info("Shutting down client...");
        }, "Shutdown-Hook"));
        
        try {
            // Load client configuration
            Configuration config;
            try {
                config = new Configuration(Paths.get("config", "client.yml"));
            } catch (IOException e) {
                LOGGER.warn("Could not load client configuration, using defaults", e);
                config = new Configuration();
            }
            
            LOGGER.info("Client starting...");
            
            // Initialize engine
            Engine engine = new Engine();
            engine.init();
            
            // TODO Phase 2: Initialize GLFW
            // TODO Phase 2: Create window
            // TODO Phase 2: Initialize OpenGL context
            // TODO Phase 2: Setup input callbacks
            // TODO Phase 2: Start engine game loop
            
            LOGGER.info("Client initialization complete");
            LOGGER.info("Press Ctrl+C to stop (game loop not yet implemented)");
            
            // Placeholder: Keep application running
            // In Phase 2, this will be replaced by the game loop
            try {
                Thread.sleep(2000);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
            
            LOGGER.info("Client stopped");
            
        } catch (Exception e) {
            LOGGER.error("Fatal error in client", e);
            System.exit(1);
        }
    }
}
