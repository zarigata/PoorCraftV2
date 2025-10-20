package com.poorcraft.client.world;

import com.poorcraft.common.config.Configuration;
import com.poorcraft.common.util.ChunkPos;
import com.poorcraft.common.world.chunk.Chunk;
import com.poorcraft.common.world.gen.TerrainGenerator;
import org.slf4j.Logger;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.*;

/**
 * Manages chunk loading/unloading based on view distance.
 */
public class ChunkLoader {
    
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(ChunkLoader.class);
    
    private final Map<ChunkPos, Chunk> loadedChunks;
    private final BlockingQueue<ChunkPos> generationQueue;
    private final BlockingQueue<Chunk> meshingQueue;
    private final ExecutorService generationThreads;
    private final ExecutorService meshingThreads;
    private final TerrainGenerator terrainGenerator;
    private final ChunkRenderer chunkRenderer;
    private final int renderDistance;
    private int lastPlayerChunkX;
    private int lastPlayerChunkZ;
    
    /**
     * Creates a chunk loader.
     */
    public ChunkLoader(Configuration config, TerrainGenerator terrainGenerator, ChunkRenderer chunkRenderer) {
        this.loadedChunks = new HashMap<>();
        this.generationQueue = new LinkedBlockingQueue<>();
        this.meshingQueue = new LinkedBlockingQueue<>();
        this.terrainGenerator = terrainGenerator;
        this.chunkRenderer = chunkRenderer;
        this.renderDistance = config.getInt("game.renderDistance", 8);
        this.lastPlayerChunkX = Integer.MAX_VALUE;
        this.lastPlayerChunkZ = Integer.MAX_VALUE;
        
        int genThreads = config.getInt("world.generationThreads", 4);
        int meshThreads = config.getInt("world.meshingThreads", 2);
        
        this.generationThreads = Executors.newFixedThreadPool(genThreads, r -> {
            Thread t = new Thread(r, "ChunkGeneration");
            t.setDaemon(true);
            return t;
        });
        
        this.meshingThreads = Executors.newFixedThreadPool(meshThreads, r -> {
            Thread t = new Thread(r, "ChunkMeshing");
            t.setDaemon(true);
            return t;
        });
        
        startWorkers();
    }
    
    /**
     * Updates chunk loading based on player position.
     */
    public void update(double playerX, double playerZ) {
        int playerChunkX = (int) Math.floor(playerX / 16.0);
        int playerChunkZ = (int) Math.floor(playerZ / 16.0);
        
        // Only update if player moved to a new chunk
        if (playerChunkX != lastPlayerChunkX || playerChunkZ != lastPlayerChunkZ) {
            updateLoadedChunks(playerChunkX, playerChunkZ);
            lastPlayerChunkX = playerChunkX;
            lastPlayerChunkZ = playerChunkZ;
        }
        
        // Process meshing results (on main thread for OpenGL)
        processMeshingResults();
    }
    
    /**
     * Updates which chunks should be loaded.
     */
    private void updateLoadedChunks(int centerX, int centerZ) {
        // Calculate required chunks
        for (int dx = -renderDistance; dx <= renderDistance; dx++) {
            for (int dz = -renderDistance; dz <= renderDistance; dz++) {
                int chunkX = centerX + dx;
                int chunkZ = centerZ + dz;
                ChunkPos pos = new ChunkPos(chunkX, chunkZ);
                
                if (!loadedChunks.containsKey(pos)) {
                    // Queue for generation
                    generationQueue.offer(pos);
                }
            }
        }
        
        // Unload distant chunks
        int unloadDistance = renderDistance + 2;
        loadedChunks.entrySet().removeIf(entry -> {
            ChunkPos pos = entry.getKey();
            int dx = pos.x() - centerX;
            int dz = pos.z() - centerZ;
            if (Math.abs(dx) > unloadDistance || Math.abs(dz) > unloadDistance) {
                chunkRenderer.removeChunk(pos.x(), pos.z());
                return true;
            }
            return false;
        });
    }
    
    /**
     * Starts worker threads.
     */
    private void startWorkers() {
        // Generation workers
        for (int i = 0; i < 4; i++) {
            generationThreads.submit(() -> {
                while (!Thread.currentThread().isInterrupted()) {
                    try {
                        ChunkPos pos = generationQueue.poll(100, TimeUnit.MILLISECONDS);
                        if (pos != null) {
                            generateChunk(pos);
                        }
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                        break;
                    }
                }
            });
        }
        
        // Meshing workers
        for (int i = 0; i < 2; i++) {
            meshingThreads.submit(() -> {
                while (!Thread.currentThread().isInterrupted()) {
                    try {
                        Chunk chunk = meshingQueue.poll(100, TimeUnit.MILLISECONDS);
                        if (chunk != null) {
                            meshChunk(chunk);
                        }
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                        break;
                    }
                }
            });
        }
    }
    
    /**
     * Generates a chunk.
     */
    private void generateChunk(ChunkPos pos) {
        try {
            Chunk chunk = new Chunk(pos.x(), pos.z());
            terrainGenerator.generateChunk(chunk);
            loadedChunks.put(pos, chunk);
            meshingQueue.offer(chunk);
        } catch (Exception e) {
            LOGGER.error("Error generating chunk {}: {}", pos, e.getMessage(), e);
        }
    }
    
    /**
     * Meshes a chunk (still on worker thread, but mesh upload happens on main thread).
     */
    private void meshChunk(Chunk chunk) {
        try {
            // Get neighbors for seamless meshing
            Chunk[] neighbors = new Chunk[4];
            neighbors[0] = loadedChunks.get(new ChunkPos(chunk.getChunkX(), chunk.getChunkZ() - 1)); // North
            neighbors[1] = loadedChunks.get(new ChunkPos(chunk.getChunkX(), chunk.getChunkZ() + 1)); // South
            neighbors[2] = loadedChunks.get(new ChunkPos(chunk.getChunkX() + 1, chunk.getChunkZ())); // East
            neighbors[3] = loadedChunks.get(new ChunkPos(chunk.getChunkX() - 1, chunk.getChunkZ())); // West
            
            ChunkMesher mesher = new ChunkMesher(chunk, neighbors);
            ChunkMesh mesh = mesher.mesh();
            
            // Store mesh for upload on main thread
            chunk.setModified(false); // Use modified flag to indicate mesh is ready
            synchronized (loadedChunks) {
                // Add to a queue for main thread processing
                pendingMeshes.put(new ChunkPos(chunk.getChunkX(), chunk.getChunkZ()), mesh);
            }
        } catch (Exception e) {
            LOGGER.error("Error meshing chunk [{}, {}]: {}", 
                        chunk.getChunkX(), chunk.getChunkZ(), e.getMessage(), e);
        }
    }
    
    private final Map<ChunkPos, ChunkMesh> pendingMeshes = new ConcurrentHashMap<>();
    
    /**
     * Processes meshing results on main thread (for OpenGL upload).
     */
    private void processMeshingResults() {
        if (pendingMeshes.isEmpty()) {
            return;
        }
        
        // Process a few meshes per frame
        int processed = 0;
        for (Map.Entry<ChunkPos, ChunkMesh> entry : pendingMeshes.entrySet()) {
            if (processed >= 5) break; // Limit per frame
            
            ChunkPos pos = entry.getKey();
            ChunkMesh mesh = entry.getValue();
            
            mesh.upload();
            
            Chunk chunk = loadedChunks.get(pos);
            if (chunk != null) {
                chunkRenderer.addChunk(chunk, mesh);
            }
            
            pendingMeshes.remove(pos);
            processed++;
        }
    }
    
    /**
     * Gets a loaded chunk.
     */
    public Chunk getChunk(int chunkX, int chunkZ) {
        return loadedChunks.get(new ChunkPos(chunkX, chunkZ));
    }
    
    /**
     * Shuts down the loader.
     */
    public void shutdown() {
        generationThreads.shutdownNow();
        meshingThreads.shutdownNow();
        try {
            generationThreads.awaitTermination(5, TimeUnit.SECONDS);
            meshingThreads.awaitTermination(5, TimeUnit.SECONDS);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
    }
}
