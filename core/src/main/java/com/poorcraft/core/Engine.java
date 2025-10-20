package com.poorcraft.core;

import com.poorcraft.common.config.Configuration;
import com.poorcraft.common.util.Logger;
import org.slf4j.Logger;

import java.io.IOException;
import java.nio.file.Paths;

/**
 * Main engine class that manages the game loop, initialization, and shutdown.
 * 
 * <p>This is a placeholder implementation for Phase 1. Phase 2 will add:
 * <ul>
 *   <li>Game loop with fixed timestep</li>
 *   <li>Resource management</li>
 *   <li>Threading and task scheduling</li>
 *   <li>Engine state management</li>
 * </ul>
 */
public class Engine {
    private static final Logger LOGGER = Logger.getLogger(Engine.class);
    
    private Configuration config;
    private boolean running;
    
    /**
     * Creates a new Engine instance.
     */
    public Engine() {
        this.running = false;
    }
    
    /**
     * Initializes the engine.
     * Loads configuration and prepares engine subsystems.
     * 
     * @throws IOException if configuration cannot be loaded
     */
    public void init() throws IOException {
        LOGGER.info("Initializing PoorCraft Engine...");
        
        // Load configuration
        try {
            config = new Configuration(Paths.get("config", "engine.yml"));
        } catch (IOException e) {
            LOGGER.warn("Could not load engine configuration, using defaults", e);
            config = new Configuration();
        }
        
        LOGGER.info("Engine initialized successfully");
    }
    
    /**
     * Starts the engine.
     * This will be expanded in Phase 2 to start the game loop.
     */
    public void start() {
        LOGGER.info("Starting engine...");
        running = true;
        
        // TODO Phase 2: Implement game loop with fixed timestep
        // TODO Phase 2: Initialize rendering subsystem
        // TODO Phase 2: Initialize resource manager
        
        LOGGER.info("Engine started");
    }
    
    /**
     * Stops the engine.
     * Performs cleanup and shutdown of all subsystems.
     */
    public void stop() {
        LOGGER.info("Stopping engine...");
        running = false;
        
        // TODO Phase 2: Cleanup resources
        // TODO Phase 2: Shutdown subsystems
        
        LOGGER.info("Engine stopped");
    }
    
    /**
     * Checks if the engine is running.
     * 
     * @return true if the engine is running
     */
    public boolean isRunning() {
        return running;
    }
    
    /**
     * Gets the engine configuration.
     * 
     * @return The configuration instance
     */
    public Configuration getConfig() {
        return config;
    }
}
