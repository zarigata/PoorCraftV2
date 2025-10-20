package com.poorcraft.core;

/**
 * Functional interface for objects that need to be rendered.
 * 
 * <p>This is called at variable frame rate for smooth rendering.
 * Objects should interpolate their visual state using alpha to eliminate jitter.
 * 
 * <p>Examples include world renderer, entity renderer, UI renderer, and particle systems.
 */
@FunctionalInterface
public interface Renderable {
    /**
     * Renders the object with interpolation.
     * 
     * @param alpha The interpolation factor (0.0 to 1.0) between previous and current simulation states
     */
    void render(double alpha);
}
