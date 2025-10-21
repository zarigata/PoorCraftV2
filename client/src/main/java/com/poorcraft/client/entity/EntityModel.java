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
    private final int textureResolution;
    private int vao;
    private int vbo;
    private int ebo;
    private boolean uploaded;

    private EntityModel(FloatBuffer vertices, IntBuffer indices, int vertexCount, int indexCount, int textureResolution) {
        this.vertices = vertices;
        this.indices = indices;
        this.vertexCount = vertexCount;
        this.indexCount = indexCount;
        this.textureResolution = textureResolution;
    }

    public static EntityModel buildPlayerModel() {
        return buildPlayerModel(32);
    }

    public static EntityModel buildPlayerModel(int textureResolution) {
        ModelBuilder builder = new ModelBuilder(textureResolution);

        builder.addBox(
                new Vector3f(-4, 12, -4),
                new Vector3f(4, 20, 4),
                UVFaces.of(
                        new UVRegion(8, 8, 8, 8),   // front
                        new UVRegion(24, 8, 8, 8),  // back
                        new UVRegion(16, 8, 8, 8),  // left
                        new UVRegion(0, 8, 8, 8),   // right
                        new UVRegion(8, 0, 8, 8),   // top
                        new UVRegion(16, 0, 8, 8)   // bottom
                )
        );

        builder.addBox(
                new Vector3f(-4, 0, -2),
                new Vector3f(4, 12, 2),
                UVFaces.of(
                        new UVRegion(8, 20, 8, 12),   // front
                        new UVRegion(20, 20, 8, 12),  // back
                        new UVRegion(16, 20, 4, 12),  // left
                        new UVRegion(4, 20, 4, 12),   // right
                        new UVRegion(8, 16, 8, 4),    // top
                        new UVRegion(16, 16, 8, 4)    // bottom
                )
        );

        UVFaces armFaces = UVFaces.of(
                new UVRegion(20, 20, 4, 12),  // front
                new UVRegion(28, 20, 4, 12),  // back
                new UVRegion(16, 20, 4, 12),  // left
                new UVRegion(24, 20, 4, 12),  // right
                new UVRegion(20, 16, 4, 4),   // top
                new UVRegion(24, 16, 4, 4)    // bottom
        );

        builder.addBox(
                new Vector3f(4, 0, -2),
                new Vector3f(8, 12, 2),
                armFaces
        );

        builder.addBox(
                new Vector3f(-8, 0, -2),
                new Vector3f(-4, 12, 2),
                armFaces
        );

        UVFaces legFaces = UVFaces.of(
                new UVRegion(4, 20, 4, 12),   // front
                new UVRegion(12, 20, 4, 12),  // back
                new UVRegion(0, 20, 4, 12),   // left
                new UVRegion(8, 20, 4, 12),   // right
                new UVRegion(4, 16, 4, 4),    // top
                new UVRegion(8, 16, 4, 4)     // bottom
        );

        builder.addBox(
                new Vector3f(0, -12, -2),
                new Vector3f(4, 0, 2),
                legFaces
        );

        builder.addBox(
                new Vector3f(-4, -12, -2),
                new Vector3f(0, 0, 2),
                legFaces
        );

        return builder.build();
    }

    public int getTextureResolution() {
        return textureResolution;
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
        private final float textureResolution;

        ModelBuilder(int textureResolution) {
            vertices = BufferUtils.createFloatBuffer(4096);
            indices = BufferUtils.createIntBuffer(4096);
            vertexOffset = 0;
            indexOffset = 0;
            this.textureResolution = textureResolution;
        }

        void addBox(Vector3f min, Vector3f max, UVFaces facesUV) {
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

            UVRegion[] regions = {
                    facesUV.front(),
                    facesUV.back(),
                    facesUV.left(),
                    facesUV.right(),
                    facesUV.top(),
                    facesUV.bottom()
            };

            for (int faceIndex = 0; faceIndex < faces.length; faceIndex++) {
                int[] face = faces[faceIndex];
                UVRegion region = regions[faceIndex];
                float[][] uvCoords = {
                        {region.u, region.v},
                        {region.u + region.w, region.v},
                        {region.u + region.w, region.v + region.h},
                        {region.u, region.v + region.h}
                };

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
                    vertices.put(uvCoord[0] / textureResolution).put(uvCoord[1] / textureResolution);
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
            return new EntityModel(vertices, indices, vertexOffset, indexOffset, (int) textureResolution);
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

    private static class UVFaces {
        private final UVRegion front;
        private final UVRegion back;
        private final UVRegion left;
        private final UVRegion right;
        private final UVRegion top;
        private final UVRegion bottom;

        private UVFaces(UVRegion front, UVRegion back, UVRegion left, UVRegion right, UVRegion top, UVRegion bottom) {
            this.front = front;
            this.back = back;
            this.left = left;
            this.right = right;
            this.top = top;
            this.bottom = bottom;
        }

        static UVFaces of(UVRegion front, UVRegion back, UVRegion left, UVRegion right, UVRegion top, UVRegion bottom) {
            return new UVFaces(front, back, left, right, top, bottom);
        }

        UVRegion front() {
            return front;
        }

        UVRegion back() {
            return back;
        }

        UVRegion left() {
            return left;
        }

        UVRegion right() {
            return right;
        }

        UVRegion top() {
            return top;
        }

        UVRegion bottom() {
            return bottom;
        }
    }
}
