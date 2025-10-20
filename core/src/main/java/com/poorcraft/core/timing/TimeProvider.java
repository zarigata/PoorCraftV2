package com.poorcraft.core.timing;

/**
 * Interface for time providing functionality.
 * Allows different implementations for different platforms (GLFW vs system time).
 */
public interface TimeProvider {
    /**
     * Gets the current time in seconds.
     * @return Current time in seconds
     */
    double getTime();
}

/**
 * System time-based time provider implementation for when GLFW is not available.
 */
class SystemTimeProvider implements TimeProvider {
    private final long startTime = System.nanoTime();

    @Override
    public double getTime() {
        return (System.nanoTime() - startTime) / 1_000_000_000.0;
    }
}
