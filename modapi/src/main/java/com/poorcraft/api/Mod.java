package com.poorcraft.api;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Annotation to mark a class as a mod entry point.
 * 
 * <p>Example usage:
 * <pre>{@code
 * @Mod(
 *     id = "examplemod",
 *     name = "Example Mod",
 *     version = "1.0.0",
 *     description = "An example mod for PoorCraft",
 *     authors = {"Author Name"}
 * )
 * public class ExampleMod {
 *     // Mod implementation
 * }
 * }</pre>
 * 
 * <p>The mod loader (Phase 6) will scan for classes with this annotation
 * and initialize them during game startup.
 */
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.TYPE)
public @interface Mod {
    
    /**
     * Unique identifier for the mod.
     * Must be lowercase, alphanumeric, and may contain underscores.
     * 
     * @return The mod ID
     */
    String id();
    
    /**
     * Human-readable name of the mod.
     * 
     * @return The mod name
     */
    String name();
    
    /**
     * Version of the mod.
     * Should follow semantic versioning (e.g., "1.0.0").
     * 
     * @return The mod version
     */
    String version();
    
    /**
     * Description of what the mod does.
     * 
     * @return The mod description
     */
    String description() default "";
    
    /**
     * Authors of the mod.
     * 
     * @return Array of author names
     */
    String[] authors() default {};
    
    /**
     * Dependencies required by this mod.
     * Format: "modid@version" (e.g., "basemod@1.0.0")
     * 
     * @return Array of dependency strings
     */
    String[] dependencies() default {};
    
    /**
     * Minimum PoorCraft version required.
     * 
     * @return Minimum game version
     */
    String minGameVersion() default "";
}
