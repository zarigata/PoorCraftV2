package com.poorcraft.client.render;

import com.poorcraft.client.render.camera.Camera;
import com.poorcraft.client.render.shader.ShaderProgram;
import com.poorcraft.client.render.texture.Texture2D;
import com.poorcraft.client.resource.ShaderManager;
import com.poorcraft.client.resource.TextureManager;
import com.poorcraft.client.window.Window;
import com.poorcraft.core.Renderable;
import org.joml.Matrix4f;
import org.lwjgl.system.MemoryStack;
import org.slf4j.Logger;

import java.nio.FloatBuffer;
import java.nio.IntBuffer;

import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.opengl.GL13.GL_TEXTURE0;
import static org.lwjgl.opengl.GL13.glActiveTexture;
import static org.lwjgl.opengl.GL15.*;
import static org.lwjgl.opengl.GL20.*;
import static org.lwjgl.opengl.GL30.*;

/**
 * Main rendering coordinator that implements the Renderable interface.
 */
public class Renderer implements Renderable {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(Renderer.class);
    
    private final Camera camera;
    private final ShaderManager shaderManager;
    private final TextureManager textureManager;
    private final Window window;
    
    // Test geometry
    private int testVAO;
    private int testVBO;
    private int testEBO;
    
    /**
     * Creates a new Renderer.
     * 
     * @param camera The camera
     * @param shaderManager Shader manager
     * @param textureManager Texture manager
     * @param window The window
     */
    public Renderer(Camera camera, ShaderManager shaderManager, TextureManager textureManager, Window window) {
        this.camera = camera;
        this.shaderManager = shaderManager;
        this.textureManager = textureManager;
        this.window = window;
    }
    
    /**
     * Initializes the renderer.
     */
    public void init() {
        LOGGER.info("Initializing renderer...");
        
        // Enable depth testing
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        
        // Enable face culling
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        
        // Enable sRGB framebuffer
        glEnable(GL_FRAMEBUFFER_SRGB);
        
        // Set clear color (sky blue)
        glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
        
        // Test geometry removed - World handles voxel rendering
        // createTestGeometry();
        
        LOGGER.info("Renderer initialized");
    }
    
    /**
     * Creates a simple textured quad for testing.
     */
    private void createTestGeometry() {
        // Vertex data: position (3), texcoord (2), normal (3)
        float[] vertices = {
            // Positions          // TexCoords  // Normals
            -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
             0.5f, -0.5f, 0.0f,   1.0f, 0.0f,   0.0f, 0.0f, 1.0f,
             0.5f,  0.5f, 0.0f,   1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
            -0.5f,  0.5f, 0.0f,   0.0f, 1.0f,   0.0f, 0.0f, 1.0f
        };
        
        // Indices
        int[] indices = {
            0, 1, 2,
            2, 3, 0
        };
        
        // Create VAO
        testVAO = glGenVertexArrays();
        glBindVertexArray(testVAO);
        
        // Create VBO
        testVBO = glGenBuffers();
        glBindBuffer(GL_ARRAY_BUFFER, testVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices, GL_STATIC_DRAW);
        
        // Create EBO
        testEBO = glGenBuffers();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, testEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices, GL_STATIC_DRAW);
        
        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, false, 8 * Float.BYTES, 0);
        glEnableVertexAttribArray(0);
        
        // TexCoord attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, false, 8 * Float.BYTES, 3 * Float.BYTES);
        glEnableVertexAttribArray(1);
        
        // Normal attribute
        glVertexAttribPointer(2, 3, GL_FLOAT, false, 8 * Float.BYTES, 5 * Float.BYTES);
        glEnableVertexAttribArray(2);
        
        glBindVertexArray(0);
        
        LOGGER.debug("Test geometry created");
    }
    
    /**
     * Handles window resize.
     * 
     * @param width New width
     * @param height New height
     */
    public void resize(int width, int height) {
        glViewport(0, 0, width, height);
        camera.setPerspective(camera.getFov(), (float) width / height, 
            camera.getNearPlane(), camera.getFarPlane());
        LOGGER.debug("Renderer resized: {}x{}", width, height);
    }
    
    @Override
    public void render(double alpha) {
        // Clear buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Update camera
        camera.update();
        
        // World handles voxel rendering
        // This renderer can be used for UI/HUD in future phases
    }
    
    /**
     * Cleans up renderer resources.
     */
    public void cleanup() {
        if (testVAO != 0) {
            glDeleteVertexArrays(testVAO);
            testVAO = 0;
        }
        if (testVBO != 0) {
            glDeleteBuffers(testVBO);
            testVBO = 0;
        }
        if (testEBO != 0) {
            glDeleteBuffers(testEBO);
            testEBO = 0;
        }
        LOGGER.info("Renderer cleaned up");
    }
}
