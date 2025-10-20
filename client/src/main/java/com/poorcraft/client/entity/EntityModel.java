package com.poorcraft.client.entity;

import org.joml.Vector3f;
import org.lwjgl.BufferUtils;

import java.nio.FloatBuffer;
import java.nio.IntBuffer;

import static org.lwjgl.opengl.GL11.GL_FLOAT;
import static org.lwjgl.opengl.GL11.GL_TRIANGLES;
import static org.lwjgl.opengl.GL11.GL_UNSIGNED_INT;
import static org.lwjgl.opengl.GL11.glDrawElements;
import static org.lwjgl.opengl.GL15.GL_ELEMENT_ARRAY_BUFFER;
import static org.lwjgl.opengl.GL15.GL_STATIC_DRAW;
import static org.lwjgl.opengl.GL15.GL_ARRAY_BUFFER;
import static org.lwjgl.opengl.GL15.glBindBuffer;
import static org.lwjgl.opengl.GL15.glBufferData;
import static org.lwjgl.opengl.GL15.glDeleteBuffers;
import static org.lwjgl.opengl.GL15.glGenBuffers;
import static org.lwjgl.opengl.GL20.glDisableVertexAttribArray;
import static org.lwjgl.opengl.GL20.glEnableVertexAttribArray;
import static org.lwjgl.opengl.GL20.glVertexAttribPointer;
import static org.lwjgl.opengl.GL30.glBindVertexArray;
import static org.lwjgl.opengl.GL30.glDeleteVertexArrays;
import static org.lwjgl.opengl.GL30.glGenVertexArrays;

public class EntityModel {
    private static final int FLOATS_PER_VERTEX = 8;
    private final FloatBuffer vertices;
    private final IntBuffer indices;
    private final int vertexCount;
    private final int indexCount;
    private int vao;
    private int vbo;
    private int ebo;
    private boolean uploaded;

    private EntityModel(FloatBuffer vertices, IntBuffer indices, int vertexCount, int indexCount) {
        this.vertices = vertices;
        this.indices = indices;
        this.vertexCount = vertexCount;
        this.indexCount = indexCount;
    }

    public static EntityModel buildPlayerModel() {
        ModelBuilder builder = new ModelBuilder();
        builder.addBox(new Vector3f(-4, 0, -2), new Vector3f(4, 12, 2), new UVRegion(16, 16, 8, 12));
        builder.addBox(new Vector3f(-4, 12, -4), new Vector3f(4, 20, 4), new UVRegion(0, 0, 8, 8));
        builder.addBox(new Vector3f(-8, 0, -2), new Vector3f(-4, 12, 2), new UVRegion(40, 16, 4, 12));
        builder.addBox(new Vector3f(4, 0, -2), new Vector3f(8, 12, 2), new UVRegion(32, 48, 4, 12));
        builder.addBox(new Vector3f(-4, -12, -2), new Vector3f(0, 0, 2), new UVRegion(0, 16, 4, 12));
        builder.addBox(new Vector3f(0, -12, -2), new Vector3f(4, 0, 2), new UVRegion(16, 48, 4, 12));
        return builder.build();
    }

    public void upload() {
        if (uploaded) {
            return;
        }
        vao = glGenVertexArrays();
        vbo = glGenBuffers();
        ebo = glGenBuffers();

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices, GL_STATIC_DRAW);

        int stride = FLOATS_PER_VERTEX * Float.BYTES;
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, false, stride, 0L);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, false, stride, 3L * Float.BYTES);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, false, stride, 5L * Float.BYTES);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        uploaded = true;
    }

    public void render() {
        if (!uploaded) {
            upload();
        }
        glBindVertexArray(vao);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0L);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
        glBindVertexArray(0);
    }

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

    private static class ModelBuilder {
        private final FloatBuffer vertices;
        private final IntBuffer indices;
        private int vertexOffset;
        private int indexOffset;

        ModelBuilder() {
            vertices = BufferUtils.createFloatBuffer(4096);
            indices = BufferUtils.createIntBuffer(4096);
            vertexOffset = 0;
            indexOffset = 0;
        }

        void addBox(Vector3f min, Vector3f max, UVRegion uv) {
            int[][] faces = {
                    {0, 1, 2, 3},
                    {4, 5, 6, 7},
                    {0, 4, 7, 3},
                    {1, 5, 6, 2},
                    {3, 2, 6, 7},
                    {0, 1, 5, 4}
            };

            Vector3f[] corners = new Vector3f[]{
                    new Vector3f(min.x, min.y, max.z),
                    new Vector3f(max.x, min.y, max.z),
                    new Vector3f(max.x, max.y, max.z),
                    new Vector3f(min.x, max.y, max.z),
                    new Vector3f(min.x, min.y, min.z),
                    new Vector3f(max.x, min.y, min.z),
                    new Vector3f(max.x, max.y, min.z),
                    new Vector3f(min.x, max.y, min.z)
            };

            float[][] uvCoords = {
                    {uv.u, uv.v}, {uv.u + uv.w, uv.v},
                    {uv.u + uv.w, uv.v + uv.h}, {uv.u, uv.v + uv.h}
            };

            for (int[] face : faces) {
                Vector3f v0 = corners[face[0]];
                Vector3f v1 = corners[face[1]];
                Vector3f v2 = corners[face[2]];

                Vector3f edge1 = new Vector3f(v1).sub(v0);
                Vector3f edge2 = new Vector3f(v2).sub(v0);
                Vector3f normal = edge1.cross(edge2).normalize();

                for (int i = 0; i < 4; i++) {
                    Vector3f vertex = corners[face[i]];
                    float[] uvCoord = uvCoords[i];
                    vertices.put(vertex.x / 16.0f).put(vertex.y / 16.0f).put(vertex.z / 16.0f);
                    vertices.put(uvCoord[0] / 64.0f).put(uvCoord[1] / 64.0f);
                    vertices.put(normal.x).put(normal.y).put(normal.z);
                }

                indices.put(vertexOffset).put(vertexOffset + 1).put(vertexOffset + 2);
                indices.put(vertexOffset + 2).put(vertexOffset + 3).put(vertexOffset);
                vertexOffset += 4;
                indexOffset += 6;
            }
        }

        EntityModel build() {
            vertices.flip();
            indices.flip();
            return new EntityModel(vertices, indices, vertexOffset, indexOffset);
        }
    }

    private static class UVRegion {
        final float u;
        final float v;
        final float w;
        final float h;

        UVRegion(float u, float v, float w, float h) {
            this.u = u;
            this.v = v;
            this.w = w;
            this.h = h;
        }
    }
}
