package com.poorcraft.core;

/**
 * Functional interface for objects that need fixed-timestep updates.
 * 
 * <p>This is called at a fixed rate (60 UPS by default) for deterministic simulation.
 * Objects implementing this interface will be registered with the Engine and updated
 * during the fixed-step loop.
 * 
 * <p>Examples include physics, game logic, AI, and world updates.
 */
@FunctionalInterface
public interface Updatable {
    /**
     * Updates the object with a fixed timestep.
     * 
     * @param dt The fixed timestep in seconds
     */
    void update(double dt);
}
