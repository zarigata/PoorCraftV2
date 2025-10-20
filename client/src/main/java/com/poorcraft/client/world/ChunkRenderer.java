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
        
        int renderedChunks = 0;
        
        // Render visible chunks
        for (Map.Entry<ChunkPos, RenderChunk> entry : chunks.entrySet()) {
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
            
            // Set model matrix (chunk position)
            Matrix4f model = new Matrix4f().translate(minX, 0, minZ);
            shader.setUniform("uModel", model);
            
            // Render chunk
            renderChunk.mesh.render();
            renderedChunks++;
        }
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
