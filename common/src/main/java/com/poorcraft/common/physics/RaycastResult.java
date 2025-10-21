package com.poorcraft.common.physics;

import org.joml.Vector3f;

import com.poorcraft.common.world.block.BlockType;

public class RaycastResult {

    public static final RaycastResult MISS = new RaycastResult(false, 0, 0, 0, 0, 0, 0, 0.0f, new Vector3f(), BlockType.AIR);

    private final boolean hit;
    private final int blockX;
    private final int blockY;
    private final int blockZ;
    private final int faceX;
    private final int faceY;
    private final int faceZ;
    private final float distance;
    private final Vector3f hitPos;
    private final BlockType blockType;

    public RaycastResult(boolean hit, int blockX, int blockY, int blockZ, int faceX, int faceY, int faceZ, float distance, Vector3f hitPos, BlockType blockType) {
        this.hit = hit;
        this.blockX = blockX;
        this.blockY = blockY;
        this.blockZ = blockZ;
        this.faceX = faceX;
        this.faceY = faceY;
        this.faceZ = faceZ;
        this.distance = distance;
        this.hitPos = hitPos;
        this.blockType = blockType;
    }

    public boolean hasHit() {
        return hit;
    }

    public int getBlockX() {
        return blockX;
    }

    public int getBlockY() {
        return blockY;
    }

    public int getBlockZ() {
        return blockZ;
    }

    public int getFaceX() {
        return faceX;
    }

    public int getFaceY() {
        return faceY;
    }

    public int getFaceZ() {
        return faceZ;
    }

    public float getDistance() {
        return distance;
    }

    public Vector3f getHitPos() {
        return hitPos;
    }

    public BlockType getBlockType() {
        return blockType;
    }

    public int[] getAdjacentBlockPos() {
        return new int[]{blockX + faceX, blockY + faceY, blockZ + faceZ};
    }
}
