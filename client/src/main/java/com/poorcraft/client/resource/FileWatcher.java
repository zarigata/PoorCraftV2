package com.poorcraft.client.resource;

import org.slf4j.Logger;

import java.io.IOException;
import java.nio.file.*;
import java.time.Duration;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;

import static java.nio.file.StandardWatchEventKinds.*;

/**
 * Monitors file system changes using Java's WatchService with debouncing.
 */
public class FileWatcher implements AutoCloseable {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(FileWatcher.class);
    
    private final WatchService watcher;
    private final Path rootPath;
    private final boolean recursive;
    private final Duration debounce;
    private final Set<String> extensions;
    private final Set<String> ignorePatterns;
    private final ReloadHandler handler;
    
    private final Thread watchThread;
    private final ScheduledExecutorService scheduler;
    private final Map<Path, ScheduledFuture<?>> pendingReloads;
    
    private volatile boolean running;
    
    /**
     * Creates a new FileWatcher.
     * 
     * @param rootPath Root directory to watch
     * @param recursive Whether to watch subdirectories
     * @param debounce Debounce duration
     * @param extensions File extensions to watch (e.g., "vert", "frag")
     * @param ignorePatterns Patterns to ignore (e.g., ".tmp", ".swp")
     * @param handler Callback for file changes
     * @throws IOException if the watcher cannot be created
     */
    public FileWatcher(Path rootPath, boolean recursive, Duration debounce, 
                      Set<String> extensions, Set<String> ignorePatterns, 
                      ReloadHandler handler) throws IOException {
        this.rootPath = rootPath;
        this.recursive = recursive;
        this.debounce = debounce;
        this.extensions = extensions;
        this.ignorePatterns = ignorePatterns;
        this.handler = handler;
        
        this.watcher = FileSystems.getDefault().newWatchService();
        this.scheduler = Executors.newScheduledThreadPool(1);
        this.pendingReloads = new HashMap<>();
        this.running = true;
        
        // Register root directory
        registerDirectory(rootPath);
        
        // Start watch thread
        this.watchThread = new Thread(this::watchLoop, "FileWatcher");
        this.watchThread.setDaemon(true);
        this.watchThread.start();
        
        LOGGER.info("File watcher started for: {} (recursive: {})", rootPath, recursive);
    }
    
    /**
     * Registers a directory with the watch service.
     */
    private void registerDirectory(Path dir) throws IOException {
        if (!Files.isDirectory(dir)) {
            return;
        }
        
        dir.register(watcher, ENTRY_CREATE, ENTRY_MODIFY, ENTRY_DELETE);
        
        // Register subdirectories if recursive
        if (recursive) {
            try (var stream = Files.walk(dir, 1)) {
                stream.filter(Files::isDirectory)
                      .filter(p -> !p.equals(dir))
                      .forEach(subDir -> {
                          try {
                              registerDirectory(subDir);
                          } catch (IOException e) {
                              LOGGER.warn("Failed to register directory: {}", subDir, e);
                          }
                      });
            }
        }
    }
    
    /**
     * Main watch loop.
     */
    private void watchLoop() {
        while (running) {
            try {
                WatchKey key = watcher.take();
                
                for (WatchEvent<?> event : key.pollEvents()) {
                    WatchEvent.Kind<?> kind = event.kind();
                    
                    if (kind == OVERFLOW) {
                        continue;
                    }
                    
                    @SuppressWarnings("unchecked")
                    WatchEvent<Path> pathEvent = (WatchEvent<Path>) event;
                    Path filename = pathEvent.context();
                    Path fullPath = rootPath.resolve(filename);
                    
                    // Filter by extension and ignore patterns
                    if (!shouldWatch(fullPath)) {
                        continue;
                    }
                    
                    // Handle directory creation for recursive watching
                    if (recursive && kind == ENTRY_CREATE && Files.isDirectory(fullPath)) {
                        registerDirectory(fullPath);
                    }
                    
                    // Debounce the reload
                    debounceReload(fullPath, kind);
                }
                
                key.reset();
                
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                break;
            } catch (Exception e) {
                LOGGER.error("Error in file watcher", e);
            }
        }
    }
    
    /**
     * Checks if a file should be watched.
     */
    private boolean shouldWatch(Path path) {
        String filename = path.getFileName().toString();
        
        // Check ignore patterns
        for (String pattern : ignorePatterns) {
            if (filename.endsWith(pattern) || filename.contains(pattern)) {
                return false;
            }
        }
        
        // Check extensions
        if (extensions.isEmpty()) {
            return true;
        }
        
        for (String ext : extensions) {
            if (filename.endsWith("." + ext)) {
                return true;
            }
        }
        
        return false;
    }
    
    /**
     * Debounces a reload event.
     */
    private void debounceReload(Path path, WatchEvent.Kind<?> kind) {
        synchronized (pendingReloads) {
            // Cancel previous scheduled reload
            ScheduledFuture<?> pending = pendingReloads.get(path);
            if (pending != null) {
                pending.cancel(false);
            }
            
            // Schedule new reload
            ScheduledFuture<?> future = scheduler.schedule(() -> {
                try {
                    Kind reloadKind = convertKind(kind);
                    handler.onReload(path, reloadKind);
                } catch (Exception e) {
                    LOGGER.error("Error handling file change: {}", path, e);
                } finally {
                    synchronized (pendingReloads) {
                        pendingReloads.remove(path);
                    }
                }
            }, debounce.toMillis(), TimeUnit.MILLISECONDS);
            
            pendingReloads.put(path, future);
        }
    }
    
    /**
     * Converts WatchEvent.Kind to ReloadHandler.Kind.
     */
    private Kind convertKind(WatchEvent.Kind<?> kind) {
        if (kind == ENTRY_CREATE) {
            return Kind.CREATE;
        } else if (kind == ENTRY_MODIFY) {
            return Kind.MODIFY;
        } else if (kind == ENTRY_DELETE) {
            return Kind.DELETE;
        }
        return Kind.MODIFY;
    }
    
    @Override
    public void close() {
        running = false;
        
        if (watchThread != null) {
            watchThread.interrupt();
        }
        
        scheduler.shutdown();
        try {
            scheduler.awaitTermination(1, TimeUnit.SECONDS);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        
        try {
            watcher.close();
        } catch (IOException e) {
            LOGGER.warn("Error closing file watcher", e);
        }
        
        LOGGER.info("File watcher closed");
    }
    
    /**
     * Callback interface for file changes.
     */
    @FunctionalInterface
    public interface ReloadHandler {
        void onReload(Path path, Kind kind);
    }
    
    /**
     * File change kind.
     */
    public enum Kind {
        CREATE, MODIFY, DELETE
    }
}
