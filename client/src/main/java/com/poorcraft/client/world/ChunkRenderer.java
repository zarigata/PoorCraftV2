package com.poorcraft.client.world;

import com.poorcraft.client.render.camera.Camera;
import com.poorcraft.client.render.camera.Frustum;
import com.poorcraft.client.render.shader.ShaderProgram;
import com.poorcraft.client.render.texture.TextureAtlas;
import com.poorcraft.common.config.Configuration;
import com.poorcraft.common.util.ChunkPos;
import com.poorcraft.common.world.chunk.Chunk;
import org.slf4j.Logger;
import org.joml.Matrix4f;
import org.joml.Vector3f;
import org.lwjgl.BufferUtils;
import org.lwjgl.opengl.GL;
import org.lwjgl.opengl.GLCapabilities;
import org.lwjgl.opengl.GL11;
import org.lwjgl.opengl.GL15;
import org.lwjgl.opengl.GL20;
import org.lwjgl.opengl.GL30;
import org.lwjgl.opengl.GL33;

import java.nio.FloatBuffer;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

/**
 * Manages rendering of all loaded chunks.
 */
public class ChunkRenderer {
    
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(ChunkRenderer.class);
    
    private final Map<ChunkPos, RenderChunk> chunks = new HashMap<>();
    private final Set<ChunkPos> renderedPositions = new HashSet<>();
    private final Camera camera;
    private final Frustum frustum;
    private final ShaderProgram shader;
    private final TextureAtlas blockAtlas;
    private final boolean occlusionQueriesSupported;
    private ShaderProgram boundingBoxShader;
    private int boundingBoxVAO;
    private int boundingBoxVBO;
    private int boundingBoxEBO;
    
    /**
     * Creates a chunk renderer.
     */
    public ChunkRenderer(Camera camera, ShaderProgram shader, TextureAtlas blockAtlas, com.poorcraft.client.resource.ShaderManager shaderManager) {
        this.camera = camera;
        this.frustum = new Frustum();
        this.shader = shader;
        this.blockAtlas = blockAtlas;
        boolean hardwareOcclusionSupported = GL.getCapabilities().GL_ARB_occlusion_query;

        // Load bounding box shader
        try {
            this.boundingBoxShader = shaderManager.loadShader("bounding_box");
        } catch (Exception e) {
            LOGGER.warn("Failed to load bounding box shader, falling back to heuristic occlusion culling", e);
            this.boundingBoxShader = null;
        }

        this.occlusionQueriesSupported = hardwareOcclusionSupported && this.boundingBoxShader != null;

        // Initialize bounding box geometry for occlusion queries
        initBoundingBoxGeometry();
    }
    
    /**
     * Initializes bounding box geometry for occlusion queries.
     * Creates a VAO/VBO/EBO for an indexed unit cube (2x2x2 centered at origin).
     */
    private void initBoundingBoxGeometry() {
        // Unit cube vertices (8 corners of a 2x2x2 cube centered at origin)
        float[] vertices = {
            // Front face
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,

            // Back face
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
        };

        // Indices for triangle faces (36 indices for 12 triangles)
        int[] indices = {
            // Front face
            0, 1, 2,  2, 3, 0,
            // Right face
            1, 5, 6,  6, 2, 1,
            // Back face
            5, 4, 7,  7, 6, 5,
            // Left face
            4, 0, 3,  3, 7, 4,
            // Top face
            3, 2, 6,  6, 7, 3,
            // Bottom face
            4, 5, 1,  1, 0, 4
        };

        // Create VAO, VBO, and EBO
        this.boundingBoxVAO = GL30.glGenVertexArrays();
        this.boundingBoxVBO = GL15.glGenBuffers();
        this.boundingBoxEBO = GL15.glGenBuffers();

        GL30.glBindVertexArray(boundingBoxVAO);

        // VBO for vertices
        GL15.glBindBuffer(GL15.GL_ARRAY_BUFFER, boundingBoxVBO);
        GL15.glBufferData(GL15.GL_ARRAY_BUFFER, vertices, GL15.GL_STATIC_DRAW);

        // EBO for indices
        GL15.glBindBuffer(GL15.GL_ELEMENT_ARRAY_BUFFER, boundingBoxEBO);
        GL15.glBufferData(GL15.GL_ELEMENT_ARRAY_BUFFER, indices, GL15.GL_STATIC_DRAW);

        // Position attribute (location 0)
        GL20.glVertexAttribPointer(0, 3, GL11.GL_FLOAT, false, 3 * Float.BYTES, 0);
        GL20.glEnableVertexAttribArray(0);

        // Unbind
        GL15.glBindBuffer(GL15.GL_ARRAY_BUFFER, 0);
        GL15.glBindBuffer(GL15.GL_ELEMENT_ARRAY_BUFFER, 0);
        GL30.glBindVertexArray(0);
    }
    
    private enum Visibility {
        UNKNOWN, VISIBLE, OCCLUDED
    }
    
    private static class AABB {
        final float minX, minY, minZ, maxX, maxY, maxZ;
        
        AABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) {
            this.minX = minX;
            this.minY = minY;
            this.minZ = minZ;
            this.maxX = maxX;
            this.maxY = maxY;
            this.maxZ = maxZ;
        }
    }
    
    private static final int MAX_UNKNOWN_PER_FRAME = 8;
    
    /**
     * Adds a chunk to be rendered.
     */
    public void addChunk(Chunk chunk, ChunkMesh mesh) {
        ChunkPos pos = new ChunkPos(chunk.getChunkX(), chunk.getChunkZ());
        RenderChunk renderChunk = new RenderChunk(chunk, mesh);
        
        // Initialize occlusion query if supported
        if (occlusionQueriesSupported) {
            renderChunk.queryId = GL15.glGenQueries();
        }
        
        chunks.put(pos, renderChunk);
    }
    
    /**
     * Removes a chunk from rendering.
     */
    public void removeChunk(int chunkX, int chunkZ) {
        ChunkPos pos = new ChunkPos(chunkX, chunkZ);
        RenderChunk renderChunk = chunks.remove(pos);
        if (renderChunk != null) {
            // Delete occlusion query object if it exists
            if (renderChunk.queryId != -1) {
                GL15.glDeleteQueries(renderChunk.queryId);
            }
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
            // Reset visibility when chunk needs rebuild
            renderChunk.visibility = Visibility.UNKNOWN;
        }
    }
    
    /**
     * Gets chunks that need rebuilding.
     * 
     * @param maxRebuilds Maximum number of chunks to return
     * @return List of chunk positions that need rebuilding
     */
    public java.util.List<ChunkPos> getChunksNeedingRebuild(int maxRebuilds) {
        java.util.List<ChunkPos> result = new java.util.ArrayList<>();
        for (Map.Entry<ChunkPos, RenderChunk> entry : chunks.entrySet()) {
            if (entry.getValue().needsRebuild) {
                result.add(entry.getKey());
                entry.getValue().needsRebuild = false;
                if (result.size() >= maxRebuilds) {
                    break;
                }
            }
        }
        return result;
    }
    
    /**
     * Updates chunks that need rebuilding.
     * Limits rebuilds per frame to avoid stuttering.
     */
    public void update(int maxRebuildsPerFrame) {
        // This is now handled by World calling getChunksNeedingRebuild
        // and routing through ChunkLoader.requestRemesh
    }
    
    /**
     * Renders all visible chunks.
     */
    public void render(Camera camera) {
        // Update frustum
        Matrix4f vp = new Matrix4f();
        camera.getProjectionMatrix().mul(camera.getViewMatrix(), vp);
        frustum.extractPlanes(vp);
        
        // Bind shader and set uniforms
        shader.use();
        shader.setMatrix4f("uView", camera.getViewMatrix());
        shader.setMatrix4f("uProjection", camera.getProjectionMatrix());
        
        // Bind texture atlas
        blockAtlas.bind(0);
        shader.setInt("uBlockAtlas", 0);
        
        // Set fog uniforms
        shader.setFloat("uFogStart", 100.0f);
        shader.setFloat("uFogEnd", 200.0f);
        shader.setVector3f("uFogColor", new Vector3f(0.5f, 0.7f, 1.0f));
        shader.setVector3f("uCameraPos", camera.getPosition());

        Vector3f sunDirection = new Vector3f(-0.3f, -1.0f, -0.2f).normalize();
        shader.setVector3f("uSunDirection", sunDirection);
        shader.setVector3f("uSunColor", new Vector3f(1.0f, 0.95f, 0.8f));
        shader.setVector3f("uAmbientColor", new Vector3f(0.4f, 0.4f, 0.5f));
        
        int renderedChunks = 0;
        int unknownChunksDrawn = 0;
        
        // Sort chunks by distance for occlusion culling (front-to-back)
        Vector3f camPos = camera.getPosition();
        java.util.List<Map.Entry<ChunkPos, RenderChunk>> sortedChunks = new java.util.ArrayList<>(chunks.entrySet());
        sortedChunks.sort((a, b) -> {
            float distA = getChunkDistanceSquared(a.getKey(), camPos);
            float distB = getChunkDistanceSquared(b.getKey(), camPos);
            return Float.compare(distA, distB);
        });
        
        // Occlusion culling pass
        if (occlusionQueriesSupported) {
            performOcclusionQueries(sortedChunks, camPos);
        } else {
            // Fallback to heuristic occlusion culling
            performHeuristicOcclusionCulling(sortedChunks, camPos);
        }
        
        // Render visible chunks
        for (Map.Entry<ChunkPos, RenderChunk> entry : sortedChunks) {
            ChunkPos pos = entry.getKey();
            RenderChunk renderChunk = entry.getValue();
            
            if (renderChunk.mesh.isEmpty()) {
                continue;
            }
            
            // Frustum culling
            if (!frustum.testAABB(
                    new Vector3f(renderChunk.bounds.minX, renderChunk.bounds.minY, renderChunk.bounds.minZ),
                    new Vector3f(renderChunk.bounds.maxX, renderChunk.bounds.maxY, renderChunk.bounds.maxZ))) {
                continue;
            }
            
            // Occlusion culling
            boolean shouldRender = shouldRenderChunk(renderChunk, unknownChunksDrawn);
            if (!shouldRender) {
                continue;
            }
            
            if (renderChunk.visibility == Visibility.VISIBLE) {
                // Set model matrix (chunk position)
                Matrix4f model = new Matrix4f().translate(renderChunk.bounds.minX, 0, renderChunk.bounds.minZ);
                shader.setMatrix4f("uModel", model);
                
                // Render chunk
                renderChunk.mesh.render();
                renderedChunks++;
                
                // Mark as rendered for heuristic occlusion culling
                if (!occlusionQueriesSupported) {
                    markChunkAsRendered(pos);
                }
            }
            
            if (renderChunk.visibility == Visibility.UNKNOWN) {
                unknownChunksDrawn++;
            }
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
     * Performs occlusion queries for all chunks that need visibility testing.
     */
    private void performOcclusionQueries(java.util.List<Map.Entry<ChunkPos, RenderChunk>> sortedChunks, Vector3f camPos) {
        // Skip if bounding box shader not available
        if (boundingBoxShader == null) {
            return;
        }

        // Enable depth testing for occlusion queries
        GL11.glEnable(GL11.GL_DEPTH_TEST);
        GL11.glColorMask(false, false, false, false); // Disable color writes
        boolean previousDepthMask = GL11.glGetBoolean(GL11.GL_DEPTH_WRITEMASK);
        GL11.glDepthMask(false);
        boolean cullFaceEnabled = GL11.glIsEnabled(GL11.GL_CULL_FACE);
        if (cullFaceEnabled) {
            GL11.glDisable(GL11.GL_CULL_FACE);
        }

        // Bind bounding box shader and set uniforms
        boundingBoxShader.use();
        boundingBoxShader.setMatrix4f("uView", camera.getViewMatrix());
        boundingBoxShader.setMatrix4f("uProjection", camera.getProjectionMatrix());

        // Bind the unit cube VAO
        GL30.glBindVertexArray(boundingBoxVAO);

        for (Map.Entry<ChunkPos, RenderChunk> entry : sortedChunks) {
            RenderChunk renderChunk = entry.getValue();

            // Only test chunks that haven't been determined to be visible
            if (renderChunk.visibility != Visibility.VISIBLE && renderChunk.queryId != -1) {
                // Calculate model matrix for this chunk's AABB
                Matrix4f model = new Matrix4f();
                float scaleX = (renderChunk.bounds.maxX - renderChunk.bounds.minX) / 2.0f;
                float scaleY = (renderChunk.bounds.maxY - renderChunk.bounds.minY) / 2.0f;
                float scaleZ = (renderChunk.bounds.maxZ - renderChunk.bounds.minZ) / 2.0f;

                model.translate(
                    renderChunk.bounds.minX + scaleX,
                    renderChunk.bounds.minY + scaleY,
                    renderChunk.bounds.minZ + scaleZ
                ).scale(scaleX, scaleY, scaleZ);

                boundingBoxShader.setMatrix4f("uModel", model);

                // Issue occlusion query for this chunk's bounding box
                GL15.glBeginQuery(GL15.GL_SAMPLES_PASSED, renderChunk.queryId);

                // Draw the unit cube as solid geometry (36 indices for 12 triangles)
                GL11.glDrawElements(GL11.GL_TRIANGLES, 36, GL11.GL_UNSIGNED_INT, 0);

                GL15.glEndQuery(GL15.GL_SAMPLES_PASSED);
            }
        }

        // Unbind VAO and restore state
        GL30.glBindVertexArray(0);
        GL11.glColorMask(true, true, true, true);
        GL11.glDepthMask(previousDepthMask);
        if (cullFaceEnabled) {
            GL11.glEnable(GL11.GL_CULL_FACE);
        }
    }
    /**
     * Updates visibility states based on occlusion query results.
     * This should be called at the beginning of each frame.
     */
    public void updateOcclusionResults() {
        if (!occlusionQueriesSupported || boundingBoxShader == null) {
            return;
        }
        
        for (RenderChunk renderChunk : chunks.values()) {
            if (renderChunk.queryId != -1 && renderChunk.visibility != Visibility.VISIBLE) {
                int available = GL15.glGetQueryObjecti(renderChunk.queryId, GL15.GL_QUERY_RESULT_AVAILABLE);
                if (available != 0) {
                    int samples = GL15.glGetQueryObjecti(renderChunk.queryId, GL15.GL_QUERY_RESULT);
                    renderChunk.visibility = (samples > 0) ? Visibility.VISIBLE : Visibility.OCCLUDED;
                }
            }
        }
    }
    
    /**
     * Performs heuristic occlusion culling for systems without hardware occlusion query support.
     */
    private void performHeuristicOcclusionCulling(java.util.List<Map.Entry<ChunkPos, RenderChunk>> sortedChunks, Vector3f camPos) {
        // Reset rendered positions for this frame
        renderedPositions.clear();
        
        // For heuristic culling, we'll mark chunks as they get rendered
        // The shouldRenderChunk method will handle the actual culling logic
    }
    
    /**
     * Marks a chunk as rendered for heuristic occlusion culling.
     */
    private void markChunkAsRendered(ChunkPos pos) {
        renderedPositions.add(pos);
    }
    
    /**
     * Determines if a chunk should be rendered based on its visibility state.
     */
    private boolean shouldRenderChunk(RenderChunk renderChunk, int unknownChunksDrawn) {
        if (occlusionQueriesSupported && boundingBoxShader != null) {
            // Hardware occlusion queries
            switch (renderChunk.visibility) {
                case VISIBLE:
                    return true;
                case OCCLUDED:
                    return false;
                case UNKNOWN:
                    // Allow a limited number of UNKNOWN chunks to be drawn per frame
                    return unknownChunksDrawn < MAX_UNKNOWN_PER_FRAME;
                default:
                    return false;
            }
        } else {
            // Heuristic occlusion culling fallback
            ChunkPos pos = new ChunkPos((int)(renderChunk.bounds.minX / 16.0f), (int)(renderChunk.bounds.minZ / 16.0f));
            return !isLikelyOccluded(pos, camera.getPosition(), renderedPositions);
        }
    }
    
    /**
     * Simple occlusion test: checks if chunk is likely occluded by closer rendered chunks.
     */
    private boolean isLikelyOccluded(ChunkPos pos, Vector3f camPos, Set<ChunkPos> renderedPositions) {
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
            renderChunk.cleanup();
        }
        chunks.clear();
        
        // Clean up bounding box geometry
        if (boundingBoxVAO != 0) {
            GL30.glDeleteVertexArrays(boundingBoxVAO);
        }
        if (boundingBoxVBO != 0) {
            GL15.glDeleteBuffers(boundingBoxVBO);
        }
        if (boundingBoxEBO != 0) {
            GL15.glDeleteBuffers(boundingBoxEBO);
        }
        if (boundingBoxShader != null) {
            boundingBoxShader.close();
        }
    }
    
    /**
     * Container for a renderable chunk.
     */
    private static class RenderChunk {
        final Chunk chunk;
        final ChunkMesh mesh;
        boolean needsRebuild;
        int queryId = -1;
        Visibility visibility = Visibility.UNKNOWN;
        AABB bounds;
        
        RenderChunk(Chunk chunk, ChunkMesh mesh) {
            this.chunk = chunk;
            this.mesh = mesh;
            this.needsRebuild = false;
            this.visibility = Visibility.UNKNOWN;
            this.bounds = new AABB(chunk.getChunkX() * 16.0f, 0.0f, chunk.getChunkZ() * 16.0f,
                                   (chunk.getChunkX() + 1) * 16.0f, 256.0f, (chunk.getChunkZ() + 1) * 16.0f);
        }
        
        void cleanup() {
            mesh.cleanup();
            if (queryId != -1) {
                GL15.glDeleteQueries(queryId);
            }
        }
    }
}
