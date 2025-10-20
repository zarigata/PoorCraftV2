package com.poorcraft.core;

import com.poorcraft.common.Constants;
import com.poorcraft.common.config.Configuration;
import com.poorcraft.core.timing.Timer;
import org.slf4j.Logger;

import java.io.IOException;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;

/**
 * Main engine class that manages the game loop, initialization, and shutdown.
 * 
 * <p>Implements Glenn Fiedler's "Fix Your Timestep" pattern for deterministic simulation
 * with smooth rendering.
 */
public class Engine {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(Engine.class);
    
    private Configuration config;
    private boolean running;
    private Timer timer;
    private double accumulator;
    
    private final List<Updatable> updatables;
    private final List<Renderable> renderables;
    
    private Runnable pollEventsCallback;
    private Runnable postRenderCallback;
    
    // Performance tracking
    private int updateCount;
    private int renderCount;
    private double lastMetricsTime;
    
    /**
     * Creates a new Engine instance.
     */
    public Engine() {
        this.running = false;
        this.accumulator = 0.0;
        this.updatables = new ArrayList<>();
        this.renderables = new ArrayList<>();
        this.updateCount = 0;
        this.renderCount = 0;
        this.lastMetricsTime = 0.0;
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
        
        // Initialize timer
        timer = new Timer();
        
        LOGGER.info("Engine initialized successfully");
    }
    
    /**
     * Starts the engine and runs the game loop.
     * Implements fixed timestep for updates and variable timestep for rendering.
     */
    public void start() {
        LOGGER.info("Starting engine game loop...");
        running = true;
        
        timer.reset();
        lastMetricsTime = timer.getTime();
        
        // Main game loop
        while (running) {
            // Get frame time and clamp to prevent spiral of death
            double frameTime = timer.getDelta();
            if (frameTime > Constants.Rendering.MAX_FRAME_TIME) {
                frameTime = Constants.Rendering.MAX_FRAME_TIME;
            }
            
            accumulator += frameTime;
            
            // Poll events (delegated to client)
            if (pollEventsCallback != null) {
                pollEventsCallback.run();
            }
            
            // Fixed timestep updates
            while (accumulator >= Constants.Rendering.FIXED_TIMESTEP) {
                update(Constants.Rendering.FIXED_TIMESTEP);
                accumulator -= Constants.Rendering.FIXED_TIMESTEP;
                updateCount++;
            }
            
            // Calculate interpolation alpha
            double alpha = accumulator / Constants.Rendering.FIXED_TIMESTEP;
            
            // Variable timestep rendering
            render(alpha);
            renderCount++;
            
            // Post-render callback (e.g., swap buffers)
            if (postRenderCallback != null) {
                postRenderCallback.run();
            }
            
            // Log performance metrics every second
            double currentTime = timer.getTime();
            if (currentTime - lastMetricsTime >= 1.0) {
                double elapsed = currentTime - lastMetricsTime;
                double ups = updateCount / elapsed;
                double fps = renderCount / elapsed;
                LOGGER.debug("Performance: {:.1f} UPS, {:.1f} FPS", ups, fps);
                updateCount = 0;
                renderCount = 0;
                lastMetricsTime = currentTime;
            }
        }
        
        LOGGER.info("Engine game loop stopped");
    }
    
    /**
     * Stops the engine.
     * Performs cleanup and shutdown of all subsystems.
     */
    public void stop() {
        LOGGER.info("Stopping engine...");
        running = false;
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
    
    /**
     * Registers an updatable object.
     * 
     * @param updatable The updatable to register
     */
    public void registerUpdatable(Updatable updatable) {
        updatables.add(updatable);
    }
    
    /**
     * Unregisters an updatable object.
     * 
     * @param updatable The updatable to unregister
     */
    public void unregisterUpdatable(Updatable updatable) {
        updatables.remove(updatable);
    }
    
    /**
     * Registers a renderable object.
     * 
     * @param renderable The renderable to register
     */
    public void registerRenderable(Renderable renderable) {
        renderables.add(renderable);
    }
    
    /**
     * Unregisters a renderable object.
     * 
     * @param renderable The renderable to unregister
     */
    public void unregisterRenderable(Renderable renderable) {
        renderables.remove(renderable);
    }
    
    /**
     * Sets the callback for polling events.
     * 
     * @param callback The callback to run before each update cycle
     */
    public void setPollEventsCallback(Runnable callback) {
        this.pollEventsCallback = callback;
    }
    
    /**
     * Sets the callback for post-render operations (e.g., buffer swapping).
     * 
     * @param callback The callback to run after rendering
     */
    public void setPostRenderCallback(Runnable callback) {
        this.postRenderCallback = callback;
    }
    
    /**
     * Updates all registered updatables with fixed timestep.
     * 
     * @param dt The fixed timestep in seconds
     */
    private void update(double dt) {
        for (Updatable updatable : updatables) {
            updatable.update(dt);
        }
    }
    
    /**
     * Renders all registered renderables with interpolation.
     * 
     * @param alpha The interpolation factor (0.0 to 1.0)
     */
    private void render(double alpha) {
        for (Renderable renderable : renderables) {
            renderable.render(alpha);
        }
    }
}
