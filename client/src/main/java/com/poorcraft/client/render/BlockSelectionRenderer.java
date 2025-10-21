package com.poorcraft.client.render;

import com.poorcraft.client.render.camera.Camera;
import com.poorcraft.client.render.shader.ShaderProgram;
import com.poorcraft.client.resource.ShaderManager;
import com.poorcraft.common.physics.RaycastResult;
import org.joml.Matrix4f;
import org.joml.Vector3f;
import org.joml.Vector4f;
import org.lwjgl.opengl.GL11;
import org.lwjgl.opengl.GL20;
import org.lwjgl.opengl.GL30;

import java.io.IOException;

import static org.lwjgl.opengl.GL11.*;

/**
 * Renders selection outline and break overlay for targeted block.
 */
public class BlockSelectionRenderer {

    private static final Vector3f OUTLINE_COLOR = new Vector3f(1.0f, 1.0f, 0.0f);
    private static final Vector3f BREAK_COLOR = new Vector3f(1.0f, 0.3f, 0.3f);
    private static final int LINE_VERTEX_COUNT = 24;
    private static final int FILL_VERTEX_COUNT = 36;

    private final Camera camera;
    private final ShaderProgram shader;
    private final int vao;
    private final int vbo;

    public BlockSelectionRenderer(Camera camera, ShaderManager shaderManager) {
        this.camera = camera;
        try {
            this.shader = shaderManager.loadShader("bounding_box");
        } catch (IOException e) {
            throw new RuntimeException("Failed to load bounding box shader", e);
        }
        int[] handles = createUnitCubeGeometry();
        this.vao = handles[0];
        this.vbo = handles[1];
    }

    public void render(RaycastResult target, float breakProgress) {
        if (target == null || !target.hasHit()) {
            return;
        }

        Matrix4f model = new Matrix4f()
                .translate(target.getBlockX(), target.getBlockY(), target.getBlockZ())
                .scale(1.001f); // slight scale to avoid z-fighting

        Vector4f outlineColor = new Vector4f(OUTLINE_COLOR, 1.0f);
        prepareRenderState(model, outlineColor, GL_LINE);
        glDrawArrays(GL_LINES, 0, LINE_VERTEX_COUNT);

        if (breakProgress > 0.0f) {
            float clamped = Math.min(1.0f, Math.max(0.0f, breakProgress));
            float alpha = 0.25f + 0.55f * clamped;
            Vector4f overlayColor = new Vector4f(BREAK_COLOR, alpha);
            prepareRenderState(model, overlayColor, GL_FILL);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDrawArrays(GL_TRIANGLES, LINE_VERTEX_COUNT, FILL_VERTEX_COUNT);
        }

        restoreState();
    }

    private void prepareRenderState(Matrix4f model, Vector4f color, int polygonMode) {
        shader.use();
        shader.setMatrix4f("uModel", model);
        shader.setMatrix4f("uView", camera.getViewMatrix());
        shader.setMatrix4f("uProjection", camera.getProjectionMatrix());
        shader.setVector4f("uColor", color);
        glBindVertexArray(vao);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(true);
        glPolygonMode(GL_FRONT_AND_BACK, polygonMode);
    }

    private void restoreState() {
        glBindVertexArray(0);
        GL20.glUseProgram(0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_BLEND);
    }

    private int[] createUnitCubeGeometry() {
        int vaoId = GL30.glGenVertexArrays();
        GL30.glBindVertexArray(vaoId);

        float[] vertices = {
            0, 0, 0,    1, 0, 0,
            1, 0, 0,    1, 1, 0,
            1, 1, 0,    0, 1, 0,
            0, 1, 0,    0, 0, 0,

            0, 0, 1,    1, 0, 1,
            1, 0, 1,    1, 1, 1,
            1, 1, 1,    0, 1, 1,
            0, 1, 1,    0, 0, 1,

            0, 0, 0,    0, 0, 1,
            1, 0, 0,    1, 0, 1,
            1, 1, 0,    1, 1, 1,
            0, 1, 0,    0, 1, 1,

            0, 0, 0,  1, 0, 0,  1, 1, 0,
            1, 1, 0,  0, 1, 0,  0, 0, 0,

            0, 0, 1,  1, 0, 1,  1, 1, 1,
            1, 1, 1,  0, 1, 1,  0, 0, 1,

            0, 0, 0,  0, 0, 1,  0, 1, 1,
            0, 1, 1,  0, 1, 0,  0, 0, 0,

            1, 0, 0,  1, 0, 1,  1, 1, 1,
            1, 1, 1,  1, 1, 0,  1, 0, 0,

            0, 0, 0,  1, 0, 0,  1, 0, 1,
            1, 0, 1,  0, 0, 1,  0, 0, 0,

            0, 1, 0,  1, 1, 0,  1, 1, 1,
            1, 1, 1,  0, 1, 1,  0, 1, 0
        };

        int vboId = GL20.glGenBuffers();
        GL20.glBindBuffer(GL20.GL_ARRAY_BUFFER, vboId);
        GL20.glBufferData(GL20.GL_ARRAY_BUFFER, vertices, GL20.GL_STATIC_DRAW);
        GL20.glEnableVertexAttribArray(0);
        GL20.glVertexAttribPointer(0, 3, GL11.GL_FLOAT, false, 0, 0);

        GL20.glBindBuffer(GL20.GL_ARRAY_BUFFER, 0);
        GL30.glBindVertexArray(0);
        return new int[]{vaoId, vboId};
    }

    public void cleanup() {
        GL20.glDeleteBuffers(vbo);
        GL30.glDeleteVertexArrays(vao);
        shader.close();
    }
}
