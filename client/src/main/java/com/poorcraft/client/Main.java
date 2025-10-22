package com.poorcraft.client;

import com.poorcraft.client.mod.ClientModManager;
import com.poorcraft.client.network.ClientNetworkManager;
import com.poorcraft.client.network.handler.ModDataHandler;
import com.poorcraft.client.network.handler.ModListHandler;
import com.poorcraft.client.ui.ConnectionUI;
import com.poorcraft.client.ui.ServerBrowserUI;
import com.poorcraft.common.event.EventBus;
import com.poorcraft.common.registry.RegistryManager;
import com.poorcraft.common.registry.Registries;
import com.poorcraft.api.ModAPI;

import java.io.IOException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.time.Duration;
import java.util.Set;
import java.util.concurrent.atomic.AtomicBoolean;

import static org.lwjgl.glfw.GLFW.GLFW_CURSOR_DISABLED;
import static org.lwjgl.glfw.GLFW.GLFW_KEY_1;
import static org.lwjgl.glfw.GLFW.GLFW_KEY_ESCAPE;

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
    private static Engine engine;
    private static Renderer renderer;
    private static World world;
    private static Entity playerEntity;
    private static ClientNetworkManager networkManager;
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

            if (multiplayer) {
                // Multiplayer mode - create network manager first
                networkManager = new ClientNetworkManager(config);
                networkManager.init();
                modManager = new ClientModManager(config, eventBus, registryManager, networkManager);
                networkManager.registerHandler(
                    com.poorcraft.common.network.packet.ModListPacket.class,
                    new ModListHandler(modManager, networkManager));
                networkManager.registerHandler(
                    com.poorcraft.common.network.packet.ModDataPacket.class,
                    new ModDataHandler(modManager));

                // Show server browser UI (placeholder implementation)
                LOGGER.info("Starting in multiplayer mode");
                ServerBrowserUI serverBrowser = new ServerBrowserUI(config, networkManager);
                String selectedServer = serverBrowser.showAndGetSelection();
                
                if (selectedServer != null) {
                    // Connect to selected server
                    ConnectionUI connectionUI = new ConnectionUI(networkManager);
                    if (connectionUI.connect(selectedServer)) {
                        LOGGER.info("Connected to server successfully");
                    } else {
                        LOGGER.error("Failed to connect to server");
                        System.exit(1);
                    }
                } else {
                    LOGGER.info("No server selected, exiting");
                    System.exit(0);
                }
            } else {
                modManager = new ClientModManager(config, eventBus, registryManager, null);
                modManager.init();
                registryManager.freezeAll();
            }

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
            Camera camera = new Camera();
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
            
            // Create world
            world = new World(config.getLong("world.seed", 0L), config, camera, shaderManager, textureManager, inputManager, networkManager);
            playerEntity = world.spawnPlayer("TestPlayer", new Vector3f(0, 80, 5));
            world.spawnNPC("TestNPC", new Vector3f(5, 80, 5), "idle");
            
            // Create renderer
            renderer = new Renderer(camera, shaderManager, textureManager, window);
            renderer.init();
            
            // Add window resize listener
            window.addResizeListener((width, height) -> renderer.resize(width, height));
            
            // Initialize engine
            engine = new Engine();
            engine.init();
            
            // Register world with engine
            engine.registerUpdatable(world);
            engine.registerRenderable(world);
            
            // Register renderer with engine (for UI/HUD in future)
            // engine.registerRenderable(renderer);
            
            // Set up event polling callback
            engine.setPollEventsCallback(() -> {
                // Clear edge states before polling new events
                inputManager.newFrame();
                
                // Poll events (triggers callbacks that set edge states)
                window.pollEvents();
                
                // Check for escape key to close
                if (inputManager.keyPressed(GLFW_KEY_ESCAPE)) {
                    engine.stop();
                }
                
                // Check if window should close
                if (window.shouldClose()) {
                    engine.stop();
                }

                if (playerEntity != null) {
                    InventoryComponent inventory = playerEntity.getComponent(InventoryComponent.class);
                    if (inventory != null) {
                        int hotbarSize = Constants.Inventory.HOTBAR_SIZE;
                        int currentSlot = inventory.getInventory().getSelectedSlot();

                        double scrollY = inputManager.getScrollY();
                        if (scrollY != 0.0d) {
                            int steps = (int) Math.round(scrollY);
                            if (steps != 0) {
                                int newSlot = Math.floorMod(currentSlot - steps, hotbarSize);
                                inventory.selectHotbarSlot(newSlot);
                                currentSlot = newSlot;
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
        if (networkManager != null) {
            networkManager.disconnect();
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
