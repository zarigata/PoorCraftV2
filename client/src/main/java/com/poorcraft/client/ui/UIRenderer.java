package com.poorcraft.client.ui;

import com.poorcraft.client.render.shader.ShaderProgram;
import com.poorcraft.client.render.texture.Texture2D;
import com.poorcraft.client.resource.ShaderManager;
import com.poorcraft.client.resource.TextureManager;
import com.poorcraft.client.window.Window;
import com.poorcraft.common.config.Configuration;
import com.poorcraft.core.Renderable;
import org.joml.Matrix4f;
import org.lwjgl.system.MemoryUtil;

import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.util.function.Consumer;

import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.opengl.GL15.*;
import static org.lwjgl.opengl.GL20.*;
import static org.lwjgl.opengl.GL30.*;

/**
 * Renders UI elements using OpenGL batching for optimal performance.
 */
public class UIRenderer implements Renderable {
    private static final int MAX_VERTICES = 65536; // 16384 quads
    private static final int MAX_INDICES = MAX_VERTICES * 6 / 4; // 6 indices per quad
    private static final int VERTEX_SIZE = 8; // pos(2) + texCoord(2) + color(4)

    private final Window window;
    private final ShaderManager shaderManager;
    private final TextureManager textureManager;
    private final Configuration config;

    private Font font;

    // OpenGL objects
    private int vao;
    private int vbo;
    private int ebo;

    private ShaderProgram uiShader;
    private ShaderProgram textShader;

    // Buffers
    private final FloatBuffer vertexBuffer;
    private final IntBuffer indexBuffer;
    private int vertexCount;
    private int indexCount;

    // Batch state
    private Texture2D currentTexture;
    private BatchType batchType;
    private float textColorR;
    private float textColorG;
    private float textColorB;
    private float textColorA;
    private Consumer<UIRenderer> renderCallback;

    // Projection matrix
    private final Matrix4f projection;
    private float uiScale;

    private enum BatchType {
        NONE,
        UI,
        TEXT
    }

    public UIRenderer(Window window, ShaderManager shaderManager, TextureManager textureManager, Configuration config) {
        this.window = window;
        this.shaderManager = shaderManager;
        this.textureManager = textureManager;
        this.config = config;
        this.projection = new Matrix4f();
        this.uiScale = config.getFloat("ui.scale", 1.0f);

        // Allocate native buffers
        if (MAX_VERTICES * VERTEX_SIZE <= 0) {
            throw new IllegalStateException("Invalid UI vertex buffer size");
        }
        this.vertexBuffer = MemoryUtil.memAllocFloat(MAX_VERTICES * VERTEX_SIZE);
        this.indexBuffer = MemoryUtil.memAllocInt(MAX_INDICES);
        this.vertexCount = 0;
        this.indexCount = 0;
        this.currentTexture = null;
        this.batchType = BatchType.NONE;
        this.textColorR = 1.0f;
        this.textColorG = 1.0f;
        this.textColorB = 1.0f;
        this.textColorA = 1.0f;

        // Generate indices for quads (0,1,2, 2,3,0)
        for (int i = 0, j = 0; i < MAX_INDICES; i += 6, j += 4) {
            indexBuffer.put(i, j);
            indexBuffer.put(i + 1, j + 1);
            indexBuffer.put(i + 2, j + 2);
            indexBuffer.put(i + 3, j + 2);
            indexBuffer.put(i + 4, j + 3);
            indexBuffer.put(i + 5, j);
        }
        indexBuffer.position(0);
    }

    public void init() {
        try {
            // Load shaders
            uiShader = shaderManager.loadShader("ui");
            textShader = shaderManager.loadShader("text");

            // Load font (may be null if loading fails)
            try {
                font = Font.fromConfig(config);
            } catch (Exception e) {
                System.err.println("Failed to load font, text rendering will be disabled: " + e.getMessage());
                font = null;
            }
        } catch (Exception e) {
            throw new RuntimeException("Failed to load UI shaders and font", e);
        }

        // Create VAO
        vao = glGenVertexArrays();
        glBindVertexArray(vao);

        // Create VBO
        vbo = glGenBuffers();
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertexBuffer.capacity() * Float.BYTES, GL_DYNAMIC_DRAW);

        // Create EBO
        ebo = glGenBuffers();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer, GL_STATIC_DRAW);

        // Setup vertex attributes
        glVertexAttribPointer(0, 2, GL_FLOAT, false, VERTEX_SIZE * Float.BYTES, 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, false, VERTEX_SIZE * Float.BYTES, 2 * Float.BYTES);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 4, GL_FLOAT, false, VERTEX_SIZE * Float.BYTES, 4 * Float.BYTES);
        glEnableVertexAttribArray(2);

        // Unbind
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // Update projection
        resize(window.getWidth(), window.getHeight());
    }

    public void beginFrame() {
        vertexBuffer.clear();
        vertexCount = 0;
        indexCount = 0;
        batchType = BatchType.NONE;
        currentTexture = null;
    }

    public void endFrame() {
        flush();
    }

    public void drawRect(float x, float y, float width, float height, int color) {
        drawTexturedRect(x, y, width, height, null, color);
    }

    public void drawTexturedRect(float x, float y, float width, float height, Texture2D texture) {
        drawTexturedRect(x, y, width, height, texture, Color.WHITE);
    }

    public void drawTexturedRect(float x, float y, float width, float height, Texture2D texture, int color) {
        // Apply UI scaling
        x *= uiScale;
        y *= uiScale;
        width *= uiScale;
        height *= uiScale;

        // Flush if switching textures or buffer full
        if (batchType != BatchType.UI || texture != currentTexture || vertexCount + 4 > MAX_VERTICES) {
            flush();
        }

        currentTexture = texture;
        batchType = BatchType.UI;

        // Calculate color components
        float r = ((color >> 16) & 0xFF) / 255.0f;
        float g = ((color >> 8) & 0xFF) / 255.0f;
        float b = (color & 0xFF) / 255.0f;
        float a = (color >>> 24) / 255.0f;

        // Add quad vertices (x, y, u, v, r, g, b, a)
        int offset = vertexCount * VERTEX_SIZE;

        // Top-left
        vertexBuffer.put(offset, x);
        vertexBuffer.put(offset + 1, y);
        vertexBuffer.put(offset + 2, 0.0f); // u
        vertexBuffer.put(offset + 3, 0.0f); // v
        vertexBuffer.put(offset + 4, r);
        vertexBuffer.put(offset + 5, g);
        vertexBuffer.put(offset + 6, b);
        vertexBuffer.put(offset + 7, a);

        // Top-right
        vertexBuffer.put(offset + 8, x + width);
        vertexBuffer.put(offset + 9, y);
        vertexBuffer.put(offset + 10, 1.0f); // u
        vertexBuffer.put(offset + 11, 0.0f); // v
        vertexBuffer.put(offset + 12, r);
        vertexBuffer.put(offset + 13, g);
        vertexBuffer.put(offset + 14, b);
        vertexBuffer.put(offset + 15, a);

        // Bottom-right
        vertexBuffer.put(offset + 16, x + width);
        vertexBuffer.put(offset + 17, y + height);
        vertexBuffer.put(offset + 18, 1.0f); // u
        vertexBuffer.put(offset + 19, 1.0f); // v
        vertexBuffer.put(offset + 20, r);
        vertexBuffer.put(offset + 21, g);
        vertexBuffer.put(offset + 22, b);
        vertexBuffer.put(offset + 23, a);

        // Bottom-left
        vertexBuffer.put(offset + 24, x);
        vertexBuffer.put(offset + 25, y + height);
        vertexBuffer.put(offset + 26, 0.0f); // u
        vertexBuffer.put(offset + 27, 1.0f); // v
        vertexBuffer.put(offset + 28, r);
        vertexBuffer.put(offset + 29, g);
        vertexBuffer.put(offset + 30, b);
        vertexBuffer.put(offset + 31, a);

        vertexCount += 4;
        indexCount += 6;
    }

    public void drawText(String text, float x, float y, int color) {
        if (text == null || text.isEmpty() || font == null) {
            return;
        }

        // Apply UI scaling
        x *= uiScale;
        y *= uiScale;

        // Flush if switching batch type or buffer full
        if (batchType != BatchType.TEXT || vertexCount + 4 > MAX_VERTICES) {
            flush();
            batchType = BatchType.TEXT;
            currentTexture = null;
            font.bindAtlas();
        }

        // Store text color for shader upload
        textColorR = ((color >> 16) & 0xFF) / 255.0f;
        textColorG = ((color >> 8) & 0xFF) / 255.0f;
        textColorB = (color & 0xFF) / 255.0f;
        textColorA = (color >>> 24) / 255.0f;

        // Render each character
        float currentX = x;
        for (int i = 0; i < text.length(); i++) {
            char c = text.charAt(i);
            if (c == '\n') {
                currentX = x;
                y += font.getLineHeight() * uiScale;
                continue;
            }

            // Get character quad data
            try (var stack = org.lwjgl.system.MemoryStack.stackPush()) {
                var quadBuffer = stack.mallocFloat(8);
                font.getCharQuad(c, quadBuffer);

                float x0 = quadBuffer.get(0);
                float y0 = quadBuffer.get(1);
                float x1 = quadBuffer.get(2);
                float y1 = quadBuffer.get(3);
                float s0 = quadBuffer.get(4);
                float t0 = quadBuffer.get(5);
                float s1 = quadBuffer.get(6);
                float t1 = quadBuffer.get(7);

                if (x0 != x1 && y0 != y1) { // Valid character
                    // Add character quad to batch
                    addTextQuad(currentX + x0 * uiScale, y + y0 * uiScale,
                                currentX + x1 * uiScale, y + y1 * uiScale,
                                s0, t0, s1, t1,
                                textColorR, textColorG, textColorB, textColorA);
                }

                currentX += font.getCharAdvance(c) * uiScale;
            }
        }
    }

    private void addTextQuad(float x0, float y0, float x1, float y1,
                           float s0, float t0, float s1, float t1,
                           float r, float g, float b, float a) {
        // Flush if buffer full
        if (vertexCount + 4 > MAX_VERTICES) {
            flush();
        }

        // Add quad vertices (x, y, u, v, r, g, b, a)
        int offset = vertexCount * VERTEX_SIZE;

        // Top-left
        vertexBuffer.put(offset, x0);
        vertexBuffer.put(offset + 1, y0);
        vertexBuffer.put(offset + 2, s0);
        vertexBuffer.put(offset + 3, t0);
        vertexBuffer.put(offset + 4, r);
        vertexBuffer.put(offset + 5, g);
        vertexBuffer.put(offset + 6, b);
        vertexBuffer.put(offset + 7, a);

        // Top-right
        vertexBuffer.put(offset + 8, x1);
        vertexBuffer.put(offset + 9, y0);
        vertexBuffer.put(offset + 10, s1);
        vertexBuffer.put(offset + 11, t0);
        vertexBuffer.put(offset + 12, r);
        vertexBuffer.put(offset + 13, g);
        vertexBuffer.put(offset + 14, b);
        vertexBuffer.put(offset + 15, a);

        // Bottom-right
        vertexBuffer.put(offset + 16, x1);
        vertexBuffer.put(offset + 17, y1);
        vertexBuffer.put(offset + 18, s1);
        vertexBuffer.put(offset + 19, t1);
        vertexBuffer.put(offset + 20, r);
        vertexBuffer.put(offset + 21, g);
        vertexBuffer.put(offset + 22, b);
        vertexBuffer.put(offset + 23, a);

        // Bottom-left
        vertexBuffer.put(offset + 24, x0);
        vertexBuffer.put(offset + 25, y1);
        vertexBuffer.put(offset + 26, s0);
        vertexBuffer.put(offset + 27, t1);
        vertexBuffer.put(offset + 28, r);
        vertexBuffer.put(offset + 29, g);
        vertexBuffer.put(offset + 30, b);
        vertexBuffer.put(offset + 31, a);

        vertexCount += 4;
        indexCount += 6;
    }

    public float measureText(String text) {
        return font != null ? font.measureText(text) : 0.0f;
    }

    public float getLineHeight() {
        return font != null ? font.getLineHeight() : 16.0f * uiScale;
    }

    public float getBaselineOffset() {
        return font != null ? font.getBaselineOffset() : 12.0f * uiScale;
    }

    private void flush() {
        if (vertexCount == 0) {
            return;
        }

        // Update VBO with new data
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        int vertexDataLength = vertexCount * VERTEX_SIZE;
        vertexBuffer.limit(vertexDataLength);
        vertexBuffer.position(0);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexBuffer);

        ShaderProgram shader;
        switch (batchType) {
            case TEXT:
                shader = textShader;
                break;
            case UI:
                shader = uiShader;
                break;
            default:
                shader = uiShader;
        }

        shader.use();
        shader.setMatrix4f("uProjection", projection);

        if (batchType == BatchType.TEXT) {
            shader.setVector4f("uTextColor", textColorR, textColorG, textColorB, textColorA);
            shader.setInt("uFontAtlas", 0);
            if (font != null) {
                font.bindAtlas();
            }
        } else {
            if (currentTexture != null) {
                shader.setInt("uHasTexture", 1);
                currentTexture.bind(0);
                shader.setInt("uTexture", 0);
            } else {
                shader.setInt("uHasTexture", 0);
            }
        }

        // Draw
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);
        vertexBuffer.clear();

        // Reset batch state
        vertexCount = 0;
        indexCount = 0;
        currentTexture = null;
        batchType = BatchType.NONE;
        vertexBuffer.limit(vertexBuffer.capacity());
    }

    public void setRenderCallback(Consumer<UIRenderer> renderCallback) {
        this.renderCallback = renderCallback;
    }

    @Override
    public void render(double alpha) {
        // Setup OpenGL state for UI rendering
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);

        beginFrame();
        if (renderCallback != null) {
            renderCallback.accept(this);
        }
        endFrame();

        // Restore state
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
    }

    public void resize(int width, int height) {
        // Update projection matrix for orthographic projection
        // Scale coordinates by UI scale factor
        float scaledWidth = width / uiScale;
        float scaledHeight = height / uiScale;

        projection.setOrtho(0.0f, scaledWidth, scaledHeight, 0.0f, -1.0f, 1.0f);
        if (uiShader != null) {
            uiShader.use();
            uiShader.setMatrix4f("uProjection", projection);
        }
        if (textShader != null) {
            textShader.use();
            textShader.setMatrix4f("uProjection", projection);
        }
        glUseProgram(0);
    }

    public void cleanup() {
        if (uiShader != null) {
            uiShader.close();
        }
        if (textShader != null) {
            textShader.close();
        }
        if (font != null) {
            font.close();
        }

        glDeleteVertexArrays(vao);
        glDeleteBuffers(vbo);
        glDeleteBuffers(ebo);

        MemoryUtil.memFree(vertexBuffer);
        MemoryUtil.memFree(indexBuffer);
    }
}
