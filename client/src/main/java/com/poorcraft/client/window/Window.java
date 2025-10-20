package com.poorcraft.client.window;

import com.poorcraft.common.Constants;
import com.poorcraft.common.config.Configuration;
import org.lwjgl.glfw.GLFWErrorCallback;
import org.lwjgl.glfw.GLFWFramebufferSizeCallback;
import org.lwjgl.glfw.GLFWVidMode;
import org.lwjgl.opengl.GL;
import org.slf4j.Logger;

import java.util.ArrayList;
import java.util.List;
import java.util.function.BiConsumer;

import static org.lwjgl.glfw.GLFW.*;
import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.system.MemoryUtil.NULL;

/**
 * Encapsulates GLFW window management and OpenGL context.
 */
public class Window implements AutoCloseable {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(Window.class);
    
    private final long handle;
    private final Configuration config;
    private final List<BiConsumer<Integer, Integer>> resizeListeners;
    
    private int width;
    private int height;
    private boolean shouldClose;
    private boolean isFullscreen;
    
    // Windowed mode state for fullscreen toggle
    private int windowedX;
    private int windowedY;
    private int windowedWidth;
    private int windowedHeight;
    
    /**
     * Creates a new Window instance.
     * 
     * @param config The configuration to use
     */
    public Window(Configuration config) {
        this.config = config;
        this.resizeListeners = new ArrayList<>();
        this.shouldClose = false;
        this.isFullscreen = false;
        
        // Initialize GLFW
        GLFWErrorCallback.createPrint(System.err).set();
        
        if (!glfwInit()) {
            throw new IllegalStateException("Failed to initialize GLFW");
        }
        
        LOGGER.info("GLFW initialized");
        
        // Configure window hints
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_RESIZABLE, config.getBoolean("window.resizable", true) ? GLFW_TRUE : GLFW_FALSE);
        
        // Parse OpenGL version from config
        int glMajor = Constants.Rendering.OPENGL_MAJOR;
        int glMinor = Constants.Rendering.OPENGL_MINOR;
        String glVersion = config.getString("graphics.openglVersion", null);
        if (glVersion != null && glVersion.contains(".")) {
            try {
                String[] parts = glVersion.split("\\.");
                glMajor = Integer.parseInt(parts[0]);
                glMinor = Integer.parseInt(parts[1]);
                LOGGER.info("Using configured OpenGL version: {}.{}", glMajor, glMinor);
            } catch (Exception e) {
                LOGGER.warn("Invalid OpenGL version format '{}', using defaults {}.{}", 
                    glVersion, glMajor, glMinor);
            }
        } else {
            LOGGER.info("Using default OpenGL version: {}.{}", glMajor, glMinor);
        }
        
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glMajor);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glMinor);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        
        // MSAA samples
        int msaaSamples = config.getInt("graphics.msaaSamples", 4);
        if (msaaSamples > 0) {
            glfwWindowHint(GLFW_SAMPLES, msaaSamples);
        }
        
        // Get window dimensions
        this.windowedWidth = config.getInt("window.width", 1280);
        this.windowedHeight = config.getInt("window.height", 720);
        this.width = windowedWidth;
        this.height = windowedHeight;
        boolean fullscreen = config.getBoolean("window.fullscreen", false);
        this.isFullscreen = fullscreen;
        String title = config.getString("window.title", "PoorCraft");
        
        // Create window
        long monitor = fullscreen ? glfwGetPrimaryMonitor() : NULL;
        this.handle = glfwCreateWindow(width, height, title, monitor, NULL);
        
        if (handle == NULL) {
            throw new RuntimeException("Failed to create GLFW window");
        }
        
        LOGGER.info("Window created: {}x{} (fullscreen: {})", width, height, fullscreen);
        
        // Center window if not fullscreen
        if (!fullscreen) {
            GLFWVidMode vidMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            if (vidMode != null) {
                windowedX = (vidMode.width() - width) / 2;
                windowedY = (vidMode.height() - height) / 2;
                glfwSetWindowPos(handle, windowedX, windowedY);
            }
        } else {
            // Store default windowed position
            windowedX = 100;
            windowedY = 100;
        }
        
        // Make context current
        glfwMakeContextCurrent(handle);
        
        // Initialize OpenGL bindings
        GL.createCapabilities();
        
        // Set swap interval (vsync)
        boolean vsync = config.getBoolean("window.vsync", true);
        glfwSwapInterval(vsync ? 1 : 0);
        
        LOGGER.info("OpenGL context created (VSync: {})", vsync);
        
        // Register callbacks
        glfwSetFramebufferSizeCallback(handle, this::onFramebufferResize);
        glfwSetWindowCloseCallback(handle, window -> shouldClose = true);
        
        // Update actual framebuffer size
        int[] fbWidth = new int[1];
        int[] fbHeight = new int[1];
        glfwGetFramebufferSize(handle, fbWidth, fbHeight);
        this.width = fbWidth[0];
        this.height = fbHeight[0];
        
        // Set initial viewport
        glViewport(0, 0, width, height);
        
        // Show window
        glfwShowWindow(handle);
    }
    
    /**
     * Checks if the window should close.
     * 
     * @return true if the window should close
     */
    public boolean shouldClose() {
        return shouldClose || glfwWindowShouldClose(handle);
    }
    
    /**
     * Polls for window events.
     */
    public void pollEvents() {
        glfwPollEvents();
    }
    
    /**
     * Swaps the front and back buffers.
     */
    public void swapBuffers() {
        glfwSwapBuffers(handle);
    }
    
    /**
     * Gets the window width.
     * 
     * @return The width in pixels
     */
    public int getWidth() {
        return width;
    }
    
    /**
     * Gets the window height.
     * 
     * @return The height in pixels
     */
    public int getHeight() {
        return height;
    }
    
    /**
     * Gets the window handle.
     * 
     * @return The GLFW window handle
     */
    public long getHandle() {
        return handle;
    }
    
    /**
     * Sets the window title.
     * 
     * @param title The new title
     */
    public void setTitle(String title) {
        glfwSetWindowTitle(handle, title);
    }
    
    /**
     * Toggles fullscreen mode.
     */
    public void toggleFullscreen() {
        isFullscreen = !isFullscreen;
        
        if (isFullscreen) {
            // Store current windowed position and size
            int[] xpos = new int[1];
            int[] ypos = new int[1];
            int[] w = new int[1];
            int[] h = new int[1];
            glfwGetWindowPos(handle, xpos, ypos);
            glfwGetWindowSize(handle, w, h);
            windowedX = xpos[0];
            windowedY = ypos[0];
            windowedWidth = w[0];
            windowedHeight = h[0];
            
            // Switch to fullscreen
            long monitor = glfwGetPrimaryMonitor();
            GLFWVidMode vidMode = glfwGetVideoMode(monitor);
            if (vidMode != null) {
                glfwSetWindowMonitor(handle, monitor, 0, 0, 
                    vidMode.width(), vidMode.height(), vidMode.refreshRate());
                LOGGER.info("Switched to fullscreen: {}x{}", vidMode.width(), vidMode.height());
            }
        } else {
            // Switch to windowed mode
            glfwSetWindowMonitor(handle, NULL, windowedX, windowedY, 
                windowedWidth, windowedHeight, GLFW_DONT_CARE);
            LOGGER.info("Switched to windowed mode: {}x{}", windowedWidth, windowedHeight);
        }
    }
    
    /**
     * Sets the cursor mode.
     * 
     * @param mode The cursor mode (GLFW_CURSOR_NORMAL, GLFW_CURSOR_HIDDEN, GLFW_CURSOR_DISABLED)
     */
    public void setCursorMode(int mode) {
        glfwSetInputMode(handle, GLFW_CURSOR, mode);
    }
    
    /**
     * Adds a resize listener.
     * 
     * @param listener The listener to add
     */
    public void addResizeListener(BiConsumer<Integer, Integer> listener) {
        resizeListeners.add(listener);
    }
    
    /**
     * Removes a resize listener.
     * 
     * @param listener The listener to remove
     */
    public void removeResizeListener(BiConsumer<Integer, Integer> listener) {
        resizeListeners.remove(listener);
    }
    
    /**
     * Framebuffer resize callback.
     */
    private void onFramebufferResize(long window, int width, int height) {
        this.width = width;
        this.height = height;
        
        glViewport(0, 0, width, height);
        
        // Notify listeners
        for (BiConsumer<Integer, Integer> listener : resizeListeners) {
            listener.accept(width, height);
        }
        
        LOGGER.debug("Framebuffer resized: {}x{}", width, height);
    }
    
    @Override
    public void close() {
        LOGGER.info("Destroying window");
        
        // Free callbacks
        GLFWFramebufferSizeCallback fbCallback = glfwSetFramebufferSizeCallback(handle, null);
        if (fbCallback != null) {
            fbCallback.free();
        }
        
        // Destroy window
        glfwDestroyWindow(handle);
        glfwTerminate();
        
        // Free error callback
        GLFWErrorCallback errorCallback = glfwSetErrorCallback(null);
        if (errorCallback != null) {
            errorCallback.free();
        }
    }
}
