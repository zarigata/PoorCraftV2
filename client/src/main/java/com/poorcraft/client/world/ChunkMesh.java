package com.poorcraft.client.world;

import org.lwjgl.BufferUtils;

import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.util.ArrayList;
import java.util.List;

import static org.lwjgl.opengl.GL30.*;

/**
 * Holds chunk mesh data and OpenGL resources.
 * Vertex format: position(3) + texCoord(2) + normal(3) + texLayer(1) + ao(1) = 10 floats
 */
public class ChunkMesh {
    
    private static final int FLOATS_PER_VERTEX = 10;
    
    private final List<Float> vertices;
    private final List<Integer> indices;
    private int vao;
    private int vbo;
    private int ebo;
    private boolean uploaded;
    
    public ChunkMesh() {
        this.vertices = new ArrayList<>();
        this.indices = new ArrayList<>();
        this.vao = 0;
        this.vbo = 0;
        this.ebo = 0;
        this.uploaded = false;
    }
    
    /**
     * Adds a quad to the mesh.
     * 
     * @param v0 First vertex [x, y, z, u, v, nx, ny, nz, texLayer, ao]
     * @param v1 Second vertex
     * @param v2 Third vertex
     * @param v3 Fourth vertex
     */
    public void addQuad(float[] v0, float[] v1, float[] v2, float[] v3) {
        int baseIndex = vertices.size() / FLOATS_PER_VERTEX;
        
        // Add vertices
        addVertex(v0);
        addVertex(v1);
        addVertex(v2);
        addVertex(v3);
        
        // Add indices for two triangles
        indices.add(baseIndex);
        indices.add(baseIndex + 1);
        indices.add(baseIndex + 2);
        
        indices.add(baseIndex);
        indices.add(baseIndex + 2);
        indices.add(baseIndex + 3);
    }
    
    private void addVertex(float[] vertex) {
        for (float v : vertex) {
            vertices.add(v);
        }
    }
    
    /**
     * Uploads mesh data to GPU.
     */
    public void upload() {
        if (vertices.isEmpty()) {
            return;
        }
        
        // Convert lists to buffers
        FloatBuffer vertexBuffer = BufferUtils.createFloatBuffer(vertices.size());
        for (Float v : vertices) {
            vertexBuffer.put(v);
        }
        vertexBuffer.flip();
        
        IntBuffer indexBuffer = BufferUtils.createIntBuffer(indices.size());
        for (Integer i : indices) {
            indexBuffer.put(i);
        }
        indexBuffer.flip();
        
        // Create VAO
        if (vao == 0) {
            vao = glGenVertexArrays();
        }
        glBindVertexArray(vao);
        
        // Create VBO
        if (vbo == 0) {
            vbo = glGenBuffers();
        }
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertexBuffer, GL_STATIC_DRAW);
        
        // Create EBO
        if (ebo == 0) {
            ebo = glGenBuffers();
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer, GL_STATIC_DRAW);
        
        // Set up vertex attributes
        int stride = FLOATS_PER_VERTEX * Float.BYTES;
        
        // Location 0: position (3 floats)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, false, stride, 0);
        
        // Location 1: texCoord (2 floats)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, false, stride, 3 * Float.BYTES);
        
        // Location 2: normal (3 floats)
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, false, stride, 5 * Float.BYTES);
        
        // Location 3: texLayer (1 float)
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, false, stride, 8 * Float.BYTES);
        
        // Location 4: ao (1 float)
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 1, GL_FLOAT, false, stride, 9 * Float.BYTES);
        
        glBindVertexArray(0);
        
        uploaded = true;
    }
    
    /**
     * Renders the mesh.
     */
    public void render() {
        if (!uploaded || indices.isEmpty()) {
            return;
        }
        
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    
    /**
     * Clears the mesh data for rebuilding.
     */
    public void clear() {
        vertices.clear();
        indices.clear();
    }
    
    /**
     * Cleans up OpenGL resources.
     */
    public void cleanup() {
        if (vao != 0) {
            glDeleteVertexArrays(vao);
            vao = 0;
        }
        if (vbo != 0) {
            glDeleteBuffers(vbo);
            vbo = 0;
        }
        if (ebo != 0) {
            glDeleteBuffers(ebo);
            ebo = 0;
        }
        uploaded = false;
    }
    
    public boolean isEmpty() {
        return vertices.isEmpty();
    }
    
    public int getVertexCount() {
        return vertices.size() / FLOATS_PER_VERTEX;
    }
    
    public int getIndexCount() {
        return indices.size();
    }
    
    public long getMemoryUsage() {
        return (long) vertices.size() * Float.BYTES + (long) indices.size() * Integer.BYTES;
    }
}
