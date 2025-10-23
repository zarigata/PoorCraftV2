package com.poorcraft.client;

import com.poorcraft.client.ui.UIManager;
import com.poorcraft.client.ui.UIRenderer;
import com.poorcraft.client.ui.UIState;
import com.poorcraft.client.window.Window;
import com.poorcraft.client.input.InputManager;
import com.poorcraft.client.render.camera.Camera;
import com.poorcraft.client.render.Renderer;
import com.poorcraft.client.render.GLInfo;
import com.poorcraft.client.resource.ShaderManager;
import com.poorcraft.client.resource.TextureManager;
import com.poorcraft.client.resource.FileWatcher;
import com.poorcraft.client.world.World;
import com.poorcraft.client.mod.ClientModManager;
import com.poorcraft.client.network.ClientNetworkManager;
import com.poorcraft.common.entity.Entity;
import com.poorcraft.common.entity.component.InventoryComponent;
import com.poorcraft.common.config.Configuration;
import com.poorcraft.common.Constants;
import com.poorcraft.common.event.EventBus;
import com.poorcraft.common.registry.RegistryManager;
import com.poorcraft.common.registry.Registries;
import com.poorcraft.api.ModAPI;
import com.poorcraft.core.Engine;
import org.joml.Vector3f;
import org.slf4j.Logger;

import java.io.IOException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.time.Duration;
import java.util.Set;
import java.util.concurrent.atomic.AtomicBoolean;

import static org.lwjgl.glfw.GLFW.GLFW_CURSOR_DISABLED;
import static org.lwjgl.glfw.GLFW.GLFW_CURSOR_NORMAL;
import static org.lwjgl.glfw.GLFW.GLFW_KEY_1;
import static org.lwjgl.glfw.GLFW.GLFW_KEY_E;
import static org.lwjgl.glfw.GLFW.GLFW_KEY_ESCAPE;
import static org.lwjgl.glfw.GLFW.GLFW_KEY_T;

/**
 * Main entry point for the PoorCraft client application.
 * 
 * <p>Phase 2 implementation with full rendering engine, game loop, and input handling.
 */
public class Main {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(Main.class);
    
    private static Window window;
    private static InputManager inputManager;
    private static ShaderManager shaderManager;
    private static TextureManager textureManager;
    private static EventBus eventBus;
    private static RegistryManager registryManager;
    private static ClientModManager modManager;
    private static FileWatcher shaderWatcher;
    private static FileWatcher textureWatcher;
    private static UIRenderer uiRenderer;
    private static UIManager uiManager;
    private static Entity playerEntity;
    private static ClientNetworkManager networkManager;
    private static World world;
    private static Renderer renderer;
    private static Engine engine;
    private static Camera camera;
    private static final AtomicBoolean cleanupExecuted = new AtomicBoolean(false);
    
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
            cleanup();
        }, "Shutdown-Hook"));
        
        try {
            // Load client configuration
            Configuration config;
            try {
                config = new Configuration(Paths.get("config", "client.yml"));
            } catch (IOException e) {
                LOGGER.warn("Could not load client configuration, using defaults", e);
                config = new Configuration();
                config.loadDefaults();
            }
            
            LOGGER.info("Client starting...");
            
            // Initialize mod infrastructure before any world construction
            eventBus = new EventBus();
            registryManager = new RegistryManager();
            Registries.init();
            ModAPI.initialize(eventBus, registryManager);

            // Check for multiplayer argument
            boolean multiplayer = args.length > 0 && "--multiplayer".equals(args[0]);

            // Initialize network manager if multiplayer
            if (multiplayer) {
                networkManager = new ClientNetworkManager(config);
                networkManager.init();
            }

            // Create window before input/world setup
            window = new Window(config);

            // Detect and log OpenGL capabilities
            GLInfo capabilities = new GLInfo();
            capabilities.printCapabilities();
            
            // Create input manager
            inputManager = new InputManager(window.getHandle(), config);
            window.setCursorMode(GLFW_CURSOR_DISABLED);
            if (config.getBoolean("input.rawMouseMotion", true)) {
                inputManager.enableRawMouseMotion(true);
            }
            
            // Create camera
            camera = new Camera();
            camera.setPosition(0.0f, 80.0f, 0.0f);
            camera.setPerspective(
                config.getFloat("game.fov", Constants.Rendering.DEFAULT_FOV),
                (float) window.getWidth() / window.getHeight(),
                Constants.Rendering.DEFAULT_NEAR_PLANE,
                Constants.Rendering.DEFAULT_FAR_PLANE
            );
            
            // Configure reverse-Z if supported and enabled
            boolean useReverseZ = config.getBoolean("graphics.useReverseZ", false);
            if (useReverseZ && capabilities.hasClipControl()) {
                camera.setReverseZ(true);
                // Set clip control for reverse-Z (requires OpenGL 4.5+)
                org.lwjgl.opengl.GL45.glClipControl(
                    org.lwjgl.opengl.GL20.GL_LOWER_LEFT,
                    org.lwjgl.opengl.GL45.GL_ZERO_TO_ONE
                );
                LOGGER.info("Reverse-Z depth enabled");
            } else if (useReverseZ) {
                LOGGER.warn("Reverse-Z requested but GL_ARB_clip_control not supported");
            }
            
            // Create resource managers
            shaderManager = new ShaderManager(config);
            textureManager = new TextureManager(config, capabilities);
            
            // Create renderer
            renderer = new Renderer(camera, shaderManager, textureManager, window);
            renderer.init();
            
            // Setup file watchers for hot-reload
            if (config.getBoolean("resources.hotReload", true)) {
                setupFileWatchers(config);
            }
            
            // Load test resources
            try {
                shaderManager.loadShader("basic");
                LOGGER.info("Loaded basic shader");
            } catch (IOException e) {
                LOGGER.error("Failed to load basic shader", e);
            }
            
            try {
                textureManager.loadTexture("test.png");
                LOGGER.info("Loaded test texture");
            } catch (IOException e) {
                LOGGER.warn("Failed to load test texture, using fallback", e);
            }
            
            // Create UI renderer
            uiRenderer = new UIRenderer(window, shaderManager, textureManager, config);
            uiRenderer.init();

            // Create UI manager
            uiManager = new UIManager(window, inputManager, networkManager, textureManager, config);
            uiManager.init();
            uiRenderer.setRenderCallback(uiManager::render);

            // Listen for UI state transitions to toggle controls/cursor
            uiManager.addStateListener((previous, current) -> {
                boolean inGame = current == UIState.IN_GAME;
                
                // Create world when transitioning to IN_GAME for the first time
                if (inGame && world == null) {
                    long seed = System.currentTimeMillis();
                    world = new World(seed, config, camera, shaderManager, textureManager, inputManager, networkManager);
                    engine.registerUpdatable(world);
                    engine.registerRenderable(world);
                    
                    // Spawn player entity
                    playerEntity = world.spawnPlayer(new Vector3f(0.0f, 80.0f, 0.0f));
                    uiManager.setPlayerEntity(playerEntity);
                    LOGGER.info("World created with seed: {}", seed);
                }
                
                if (world != null) {
                    world.setPlayerControlsEnabled(inGame);
                }
                int cursorMode = inGame ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL;
                window.setCursorMode(cursorMode);
            });

            uiManager.setState(UIState.MAIN_MENU);
            
            // Add window resize listener
            window.addResizeListener((width, height) -> {
                renderer.resize(width, height);
                uiRenderer.resize(width, height);
                uiManager.resize(width, height);
            });
            
            // Initialize engine
            engine = new Engine();
            engine.init();
            
            // Register renderer and UI with engine
            engine.registerRenderable(renderer);
            engine.registerUpdatable(uiManager);
            engine.registerRenderable(uiRenderer);
            
            // Set up event polling callback
            engine.setPollEventsCallback(() -> {
                // Clear edge states before polling new events
                inputManager.newFrame();

                // Poll events (triggers callbacks that set edge states)
                window.pollEvents();

                // Handle UI input first - if UI consumes input, don't process game input
                boolean uiConsumedInput = uiManager.handleInput(inputManager);

                if (!uiConsumedInput) {
                    // Process game input only if UI didn't consume it
                    handleGameInput();
                }

                // Check if window should close
                if (window.shouldClose()) {
                    engine.stop();
                }
            });
            
            // Set up post-render callback to swap buffers
            engine.setPostRenderCallback(window::swapBuffers);
            
            LOGGER.info("Client initialization complete");
            LOGGER.info("Press ESC to exit");
            
            // Start engine game loop (blocking)
            engine.start();
            
            LOGGER.info("Client stopped");
            cleanup();
            
        } catch (Exception e) {
            LOGGER.error("Fatal error in client", e);
            cleanup();
            System.exit(1);
        }
    }

    /**
     * Handles game-specific input when UI doesn't consume it.
     */
    private static void handleGameInput() {
        UIState currentState = uiManager.getCurrentState();

        // Handle state-specific input
        switch (currentState) {
            case IN_GAME:
                handleInGameInput();
                break;
            case PAUSED:
                handlePausedInput();
                break;
            case INVENTORY:
                handleInventoryInput();
                break;
            default:
                // Other states handled by UI system
                break;
        }
    }

    private static void handleInGameInput() {
        // Handle in-game input (hotbar, inventory toggle, etc.)
        if (playerEntity != null) {
            InventoryComponent inventory = playerEntity.getComponent(InventoryComponent.class);
            if (inventory != null) {
                handleHotbarInput(inventory);
            }
        }

        // Handle UI state transitions
        if (inputManager.keyPressed(GLFW_KEY_ESCAPE)) {
            uiManager.setState(UIState.PAUSED);
        }
        if (inputManager.keyPressed(GLFW_KEY_E)) {
            uiManager.setState(UIState.INVENTORY);
        }
        if (inputManager.keyPressed(GLFW_KEY_T)) {
            uiManager.setState(UIState.CHAT);
        }
    }

    private static void handlePausedInput() {
        // Handle pause menu input
        if (inputManager.keyPressed(GLFW_KEY_ESCAPE)) {
            uiManager.setState(UIState.IN_GAME);
        }
    }

    private static void handleInventoryInput() {
        // Handle inventory input
        if (inputManager.keyPressed(GLFW_KEY_ESCAPE) || inputManager.keyPressed(GLFW_KEY_E)) {
            uiManager.setState(UIState.IN_GAME);
        }
    }

    private static void handleHotbarInput(InventoryComponent inventory) {
        int hotbarSize = Constants.Inventory.HOTBAR_SIZE;
        int currentSlot = inventory.getInventory().getSelectedSlot();

        double scrollY = inputManager.getScrollY();
        if (scrollY != 0.0d) {
            int steps = (int) Math.round(scrollY);
            if (steps != 0) {
                int newSlot = Math.floorMod(currentSlot - steps, hotbarSize);
                inventory.selectHotbarSlot(newSlot);
            }
        }

        for (int i = 0; i < Math.min(hotbarSize, 9); i++) {
            int keyCode = GLFW_KEY_1 + i;
            if (inputManager.keyPressed(keyCode)) {
                inventory.selectHotbarSlot(i);
                break;
            }
        }
    }
    
    /**
     * Sets up file watchers for hot-reloading resources.
     */
    private static void setupFileWatchers(Configuration config) {
        try {
            int debounceMs = config.getInt("resources.watchDebounceMs", 150);
            Duration debounce = Duration.ofMillis(debounceMs);
            
            // Shader watcher
            Path shaderPath = Path.of(config.getString("resources.shaderPath", "shaders"));
            shaderWatcher = new FileWatcher(
                shaderPath,
                false,
                debounce,
                Set.of("vert", "frag", "glsl"),
                Set.of(".tmp", ".swp", ".DS_Store", "~"),
                (path, kind) -> {
                    LOGGER.info("Shader file changed: {} ({})", path, kind);
                    // Trigger shader reload
                    String filename = path.getFileName().toString();
                    String baseName = filename.substring(0, filename.lastIndexOf('.'));
                    try {
                        shaderManager.reloadShader(baseName);
                    } catch (IOException e) {
                        LOGGER.error("Failed to reload shader: {}", baseName, e);
                    }
                }
            );
            
            // Texture watcher
            Path texturePath = Path.of(config.getString("resources.texturePath", "textures"));
            textureWatcher = new FileWatcher(
                texturePath,
                false,
                debounce,
                Set.of("png", "jpg", "jpeg"),
                Set.of(".tmp", ".swp", ".DS_Store", "~"),
                (path, kind) -> {
                    LOGGER.info("Texture file changed: {} ({})", path, kind);
                    // Trigger texture reload
                    String filename = path.getFileName().toString();
                    try {
                        textureManager.reloadTexture(filename);
                    } catch (IOException e) {
                        LOGGER.error("Failed to reload texture: {}", filename, e);
                    }
                }
            );
            
            LOGGER.info("Hot-reload file watchers enabled");
        } catch (IOException e) {
            LOGGER.warn("Failed to setup file watchers, hot-reload disabled", e);
        }
    }
    
    /**
     * Cleans up all resources.
     */
    private static void cleanup() {
        // Guard against double cleanup
        if (!cleanupExecuted.compareAndSet(false, true)) {
            return;
        }
        if (uiManager != null) {
            uiManager.cleanup();
        }
        if (uiRenderer != null) {
            uiRenderer.cleanup();
        }
        if (world != null) {
            world.cleanup();
        }
        if (renderer != null) {
            renderer.cleanup();
        }
        if (shaderWatcher != null) {
            shaderWatcher.close();
        }
        if (textureWatcher != null) {
            textureWatcher.close();
        }
        if (shaderManager != null) {
            shaderManager.close();
        }
        if (textureManager != null) {
            textureManager.close();
        }
        if (inputManager != null) {
            inputManager.close();
        }
        if (window != null) {
            window.close();
        }
        LOGGER.info("Cleanup complete");
    }
}
