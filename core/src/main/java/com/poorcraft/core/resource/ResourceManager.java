package com.poorcraft.core.resource;

import org.slf4j.Logger;

import java.io.IOException;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.function.Consumer;

/**
 * Abstract base class for resource management with hot-reload support.
 * 
 * @param <T> The type of resource managed
 */
public abstract class ResourceManager<T> implements AutoCloseable {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(ResourceManager.class);
    
    protected final Map<String, T> resources;
    protected final Map<String, Consumer<T>> reloadListeners;
    
    /**
     * Creates a new ResourceManager instance.
     */
    public ResourceManager() {
        this.resources = new ConcurrentHashMap<>();
        this.reloadListeners = new ConcurrentHashMap<>();
    }
    
    /**
     * Loads a resource from the specified path.
     * 
     * @param path The resource path (relative name)
     * @return The loaded resource
     * @throws IOException if the resource cannot be loaded
     */
    public T load(String path) throws IOException {
        if (resources.containsKey(path)) {
            LOGGER.debug("Resource already loaded: {}", path);
            return resources.get(path);
        }
        
        T resource = loadResource(resolvePath(path));
        resources.put(path, resource);
        LOGGER.info("Loaded resource: {}", path);
        return resource;
    }
    
    /**
     * Reloads an existing resource.
     * 
     * @param path The resource path
     * @throws IOException if the resource cannot be reloaded
     */
    public void reload(String path) throws IOException {
        T oldResource = resources.get(path);
        if (oldResource == null) {
            LOGGER.warn("Cannot reload non-existent resource: {}", path);
            return;
        }
        
        try {
            T newResource = loadResource(resolvePath(path));
            unloadResource(oldResource);
            resources.put(path, newResource);
            
            // Notify listeners
            Consumer<T> listener = reloadListeners.get(path);
            if (listener != null) {
                listener.accept(newResource);
            }
            
            LOGGER.info("Reloaded resource: {}", path);
        } catch (IOException e) {
            LOGGER.error("Failed to reload resource: {}", path, e);
            throw e;
        }
    }
    
    /**
     * Unloads a resource.
     * 
     * @param path The resource path
     */
    public void unload(String path) {
        T resource = resources.remove(path);
        if (resource != null) {
            unloadResource(resource);
            reloadListeners.remove(path);
            LOGGER.info("Unloaded resource: {}", path);
        }
    }
    
    /**
     * Gets a loaded resource.
     * 
     * @param path The resource path
     * @return The resource, or null if not loaded
     */
    public T get(String path) {
        return resources.get(path);
    }
    
    /**
     * Gets all loaded resources.
     * 
     * @return Map of all loaded resources
     */
    public Map<String, T> getAll() {
        return new ConcurrentHashMap<>(resources);
    }
    
    /**
     * Registers a reload listener for a resource.
     * 
     * @param path The resource path
     * @param listener The listener to call when the resource is reloaded
     */
    public void registerReloadListener(String path, Consumer<T> listener) {
        reloadListeners.put(path, listener);
    }
    
    /**
     * Loads a resource from an absolute path.
     * Must be implemented by subclasses.
     * 
     * @param absolutePath The absolute path to the resource
     * @return The loaded resource
     * @throws IOException if the resource cannot be loaded
     */
    protected abstract T loadResource(Path absolutePath) throws IOException;
    
    /**
     * Unloads a resource and releases its resources.
     * Must be implemented by subclasses.
     * 
     * @param resource The resource to unload
     */
    protected abstract void unloadResource(T resource);
    
    /**
     * Resolves a relative path to an absolute path.
     * Can be overridden by subclasses to customize path resolution.
     * 
     * @param path The relative path
     * @return The absolute path
     */
    protected Path resolvePath(String path) {
        return Path.of(path);
    }
    
    @Override
    public void close() {
        LOGGER.info("Closing resource manager, unloading {} resources", resources.size());
        // Copy keys to avoid concurrent modification
        for (String path : new ArrayList<>(resources.keySet())) {
            unload(path);
        }
        resources.clear();
        reloadListeners.clear();
    }
}
