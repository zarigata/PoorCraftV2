package com.poorcraft.common.physics;

import org.joml.Vector3f;

/**
 * Axis-aligned bounding box used for collision detection.
 */
public class AABB {

    private float minX;
    private float minY;
    private float minZ;
    private float maxX;
    private float maxY;
    private float maxZ;

    public AABB(Vector3f min, Vector3f max) {
        this(min.x, min.y, min.z, max.x, max.y, max.z);
    }

    public AABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) {
        this.minX = Math.min(minX, maxX);
        this.minY = Math.min(minY, maxY);
        this.minZ = Math.min(minZ, maxZ);
        this.maxX = Math.max(minX, maxX);
        this.maxY = Math.max(minY, maxY);
        this.maxZ = Math.max(minZ, maxZ);
    }

    public boolean intersects(AABB other) {
        return this.maxX > other.minX && this.minX < other.maxX
            && this.maxY > other.minY && this.minY < other.maxY
            && this.maxZ > other.minZ && this.minZ < other.maxZ;
    }

    public boolean contains(Vector3f point) {
        return point.x >= minX && point.x <= maxX
            && point.y >= minY && point.y <= maxY
            && point.z >= minZ && point.z <= maxZ;
    }

    public AABB expand(float amount) {
        minX -= amount;
        minY -= amount;
        minZ -= amount;
        maxX += amount;
        maxY += amount;
        maxZ += amount;
        return this;
    }

    public AABB offset(Vector3f delta) {
        return offset(delta.x, delta.y, delta.z);
    }

    public AABB offset(float dx, float dy, float dz) {
        minX += dx;
        minY += dy;
        minZ += dz;
        maxX += dx;
        maxY += dy;
        maxZ += dz;
        return this;
    }

    public AABB union(AABB other) {
        return new AABB(
            Math.min(this.minX, other.minX),
            Math.min(this.minY, other.minY),
            Math.min(this.minZ, other.minZ),
            Math.max(this.maxX, other.maxX),
            Math.max(this.maxY, other.maxY),
            Math.max(this.maxZ, other.maxZ)
        );
    }

    public Vector3f getCenter() {
        return new Vector3f(
            (minX + maxX) * 0.5f,
            (minY + maxY) * 0.5f,
            (minZ + maxZ) * 0.5f
        );
    }

    public Vector3f getSize() {
        return new Vector3f(
            maxX - minX,
            maxY - minY,
            maxZ - minZ
        );
    }

    public static AABB fromEntity(Vector3f position, float width, float height) {
        float halfWidth = width * 0.5f;
        return new AABB(
            position.x - halfWidth,
            position.y,
            position.z - halfWidth,
            position.x + halfWidth,
            position.y + height,
            position.z + halfWidth
        );
    }

    public float getMinX() {
        return minX;
    }

    public float getMinY() {
        return minY;
    }

    public float getMinZ() {
        return minZ;
    }

    public float getMaxX() {
        return maxX;
    }

    public float getMaxY() {
        return maxY;
    }

    public float getMaxZ() {
        return maxZ;
    }

    public void setMinX(float minX) {
        this.minX = minX;
    }

    public void setMinY(float minY) {
        this.minY = minY;
    }

    public void setMinZ(float minZ) {
        this.minZ = minZ;
    }

    public void setMaxX(float maxX) {
        this.maxX = maxX;
    }

    public void setMaxY(float maxY) {
        this.maxY = maxY;
    }

    public void setMaxZ(float maxZ) {
        this.maxZ = maxZ;
    }

    @Override
    public String toString() {
        return "AABB{" +
            "minX=" + minX +
            ", minY=" + minY +
            ", minZ=" + minZ +
            ", maxX=" + maxX +
            ", maxY=" + maxY +
            ", maxZ=" + maxZ +
            '}';
    }
}
