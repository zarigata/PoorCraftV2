package com.poorcraft.client.world;

import com.poorcraft.client.render.camera.Camera;
import com.poorcraft.client.render.camera.Frustum;
import com.poorcraft.client.render.shader.ShaderProgram;
import com.poorcraft.client.render.texture.TextureAtlas;
import com.poorcraft.common.util.ChunkPos;
import com.poorcraft.common.world.chunk.Chunk;
import org.joml.Matrix4f;
import org.joml.Vector3f;

import java.util.HashMap;
import java.util.Map;

/**
 * Manages rendering of all loaded chunks.
 */
public class ChunkRenderer {
    
    private final Map<ChunkPos, RenderChunk> chunks;
    private final Camera camera;
    private final Frustum frustum;
    private final ShaderProgram shader;
    private final TextureAtlas blockAtlas;
    
    /**
     * Creates a chunk renderer.
     */
    public ChunkRenderer(Camera camera, ShaderProgram shader, TextureAtlas blockAtlas) {
        this.chunks = new HashMap<>();
        this.camera = camera;
        this.frustum = new Frustum();
        this.shader = shader;
        this.blockAtlas = blockAtlas;
    }
    
    /**
     * Adds a chunk to be rendered.
     */
    public void addChunk(Chunk chunk, ChunkMesh mesh) {
        ChunkPos pos = new ChunkPos(chunk.getChunkX(), chunk.getChunkZ());
        RenderChunk renderChunk = new RenderChunk(chunk, mesh);
        chunks.put(pos, renderChunk);
    }
    
    /**
     * Removes a chunk from rendering.
     */
    public void removeChunk(int chunkX, int chunkZ) {
        ChunkPos pos = new ChunkPos(chunkX, chunkZ);
        RenderChunk renderChunk = chunks.remove(pos);
        if (renderChunk != null) {
            renderChunk.mesh.cleanup();
        }
    }
    
    /**
     * Marks a chunk for rebuild.
     */
    public void markForRebuild(int chunkX, int chunkZ) {
        ChunkPos pos = new ChunkPos(chunkX, chunkZ);
        RenderChunk renderChunk = chunks.get(pos);
        if (renderChunk != null) {
            renderChunk.needsRebuild = true;
        }
    }
    
    /**
     * Updates chunks that need rebuilding.
     * Limits rebuilds per frame to avoid stuttering.
     */
    public void update(int maxRebuildsPerFrame) {
        int rebuilds = 0;
        for (RenderChunk renderChunk : chunks.values()) {
            if (renderChunk.needsRebuild && rebuilds < maxRebuildsPerFrame) {
                // Rebuild would happen here in a more complete implementation
                // For now, just mark as not needing rebuild
                renderChunk.needsRebuild = false;
                rebuilds++;
            }
        }
    }
    
    /**
     * Renders all visible chunks.
     */
    public void render(Camera camera) {
        // Update frustum
        Matrix4f vp = new Matrix4f();
        camera.getProjectionMatrix().mul(camera.getViewMatrix(), vp);
        frustum.update(vp);
        
        // Bind shader and set uniforms
        shader.use();
        shader.setUniform("uView", camera.getViewMatrix());
        shader.setUniform("uProjection", camera.getProjectionMatrix());
        
        // Bind texture atlas
        blockAtlas.bind(0);
        shader.setUniform("uBlockAtlas", 0);
        
        // Set lighting uniforms
        shader.setUniform("uSunDirection", new Vector3f(0.5f, 1.0f, 0.3f).normalize());
        shader.setUniform("uSunColor", new Vector3f(1.0f, 0.95f, 0.8f));
        shader.setUniform("uAmbientColor", new Vector3f(0.4f, 0.4f, 0.5f));
        
        // Set fog uniforms
        shader.setUniform("uFogStart", 100.0f);
        shader.setUniform("uFogEnd", 200.0f);
        shader.setUniform("uFogColor", new Vector3f(0.5f, 0.7f, 1.0f));
        shader.setUniform("uCameraPos", camera.getPosition());
        
        int renderedChunks = 0;
        
        // Sort chunks by distance for occlusion culling (front-to-back)
        Vector3f camPos = camera.getPosition();
        java.util.List<Map.Entry<ChunkPos, RenderChunk>> sortedChunks = new java.util.ArrayList<>(chunks.entrySet());
        sortedChunks.sort((a, b) -> {
            float distA = getChunkDistanceSquared(a.getKey(), camPos);
            float distB = getChunkDistanceSquared(b.getKey(), camPos);
            return Float.compare(distA, distB);
        });
        
        // Simple occlusion culling: track rendered chunk positions
        // More sophisticated approach would use hardware occlusion queries
        java.util.Set<ChunkPos> renderedPositions = new java.util.HashSet<>();
        
        // Render visible chunks
        for (Map.Entry<ChunkPos, RenderChunk> entry : sortedChunks) {
            ChunkPos pos = entry.getKey();
            RenderChunk renderChunk = entry.getValue();
            
            if (renderChunk.mesh.isEmpty()) {
                continue;
            }
            
            // Frustum culling
            float minX = pos.x() * 16.0f;
            float minY = 0.0f;
            float minZ = pos.z() * 16.0f;
            float maxX = minX + 16.0f;
            float maxY = 256.0f;
            float maxZ = minZ + 16.0f;
            
            if (!frustum.testAABB(minX, minY, minZ, maxX, maxY, maxZ)) {
                continue;
            }
            
            // Simple occlusion culling: skip chunks that are likely occluded
            // by closer chunks that have already been rendered
            if (isLikelyOccluded(pos, camPos, renderedPositions)) {
                continue;
            }
            
            // Set model matrix (chunk position)
            Matrix4f model = new Matrix4f().translate(minX, 0, minZ);
            shader.setUniform("uModel", model);
            
            // Render chunk
            renderChunk.mesh.render();
            renderedChunks++;
            renderedPositions.add(pos);
        }
    }
    
    /**
     * Calculates squared distance from camera to chunk center.
     */
    private float getChunkDistanceSquared(ChunkPos pos, Vector3f camPos) {
        float chunkCenterX = pos.x() * 16.0f + 8.0f;
        float chunkCenterZ = pos.z() * 16.0f + 8.0f;
        float dx = chunkCenterX - camPos.x;
        float dz = chunkCenterZ - camPos.z;
        return dx * dx + dz * dz;
    }
    
    /**
     * Simple occlusion test: checks if chunk is likely occluded by closer rendered chunks.
     */
    private boolean isLikelyOccluded(ChunkPos pos, Vector3f camPos, java.util.Set<ChunkPos> renderedPositions) {
        // Get direction from camera to chunk
        float chunkCenterX = pos.x() * 16.0f + 8.0f;
        float chunkCenterZ = pos.z() * 16.0f + 8.0f;
        float dx = chunkCenterX - camPos.x;
        float dz = chunkCenterZ - camPos.z;
        float dist = (float) Math.sqrt(dx * dx + dz * dz);
        
        if (dist < 32.0f) {
            return false; // Don't occlude very close chunks
        }
        
        // Normalize direction
        float ndx = dx / dist;
        float ndz = dz / dist;
        
        // Check if there's a rendered chunk in front of this one along the view direction
        for (int step = 1; step < 4; step++) {
            int testX = (int) Math.floor((camPos.x + ndx * step * 16.0f) / 16.0f);
            int testZ = (int) Math.floor((camPos.z + ndz * step * 16.0f) / 16.0f);
            ChunkPos testPos = new ChunkPos(testX, testZ);
            
            if (testPos.equals(pos)) {
                break;
            }
            
            if (renderedPositions.contains(testPos)) {
                return true; // Likely occluded by this chunk
            }
        }
        
        return false;
    }
    
    /**
     * Gets the number of visible chunks rendered last frame.
     */
    public int getVisibleChunkCount() {
        return chunks.size();
    }
    
    /**
     * Cleans up all chunk meshes.
     */
    public void cleanup() {
        for (RenderChunk renderChunk : chunks.values()) {
            renderChunk.mesh.cleanup();
        }
        chunks.clear();
    }
    
    /**
     * Container for a renderable chunk.
     */
    private static class RenderChunk {
        final Chunk chunk;
        final ChunkMesh mesh;
        boolean needsRebuild;
        
        RenderChunk(Chunk chunk, ChunkMesh mesh) {
            this.chunk = chunk;
            this.mesh = mesh;
            this.needsRebuild = false;
        }
    }
}
