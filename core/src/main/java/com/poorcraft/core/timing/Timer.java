package com.poorcraft.core.timing;

import org.lwjgl.glfw.GLFW;

/**
 * High-resolution timer utility using GLFW's glfwGetTime() for consistent cross-platform timing.
 */
public class Timer {
    private double lastTime;
    private double deltaTime;
    private int frameCount;
    private double fpsTime;
    private double fps;
    
    /**
     * Creates a new Timer instance.
     */
    public Timer() {
        this.lastTime = GLFW.glfwGetTime();
        this.deltaTime = 0.0;
        this.frameCount = 0;
        this.fpsTime = 0.0;
        this.fps = 0.0;
    }
    
    /**
     * Gets the current time in seconds.
     * 
     * @return Current time in seconds
     */
    public double getTime() {
        return GLFW.glfwGetTime();
    }
    
    /**
     * Gets the time since the last getDelta() call.
     * Updates internal state for next call.
     * 
     * @return Delta time in seconds
     */
    public double getDelta() {
        double currentTime = GLFW.glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        
        // Update FPS calculation
        frameCount++;
        fpsTime += deltaTime;
        
        if (fpsTime >= 1.0) {
            fps = frameCount / fpsTime;
            frameCount = 0;
            fpsTime = 0.0;
        }
        
        return deltaTime;
    }
    
    /**
     * Resets the timer.
     */
    public void reset() {
        lastTime = GLFW.glfwGetTime();
        deltaTime = 0.0;
        frameCount = 0;
        fpsTime = 0.0;
        fps = 0.0;
    }
    
    /**
     * Gets the current frames per second.
     * 
     * @return FPS value
     */
    public double getFPS() {
        return fps;
    }
    
    /**
     * Formats time as a human-readable string.
     * 
     * @param seconds Time in seconds
     * @return Formatted string (e.g., "1.234s")
     */
    public static String formatTime(double seconds) {
        return String.format("%.3fs", seconds);
    }
}
