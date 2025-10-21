package com.poorcraft.common.physics;

import java.util.function.Predicate;

import org.joml.Vector3f;

import com.poorcraft.common.world.World;
import com.poorcraft.common.world.block.BlockType;

public final class Raycast {

    private Raycast() {
    }

    public static RaycastResult raycast(Vector3f origin, Vector3f direction, float maxDistance, World world) {
        return raycastBlocks(origin, direction, maxDistance, world, BlockType::isSolid);
    }

    public static RaycastResult raycastBlocks(Vector3f origin, Vector3f direction, float maxDistance, World world, Predicate<BlockType> filter) {
        if (direction.lengthSquared() == 0.0f) {
            return RaycastResult.MISS;
        }

        Vector3f dir = new Vector3f(direction).normalize();

        int blockX = (int) Math.floor(origin.x);
        int blockY = (int) Math.floor(origin.y);
        int blockZ = (int) Math.floor(origin.z);

        int stepX = dir.x > 0.0f ? 1 : (dir.x < 0.0f ? -1 : 0);
        int stepY = dir.y > 0.0f ? 1 : (dir.y < 0.0f ? -1 : 0);
        int stepZ = dir.z > 0.0f ? 1 : (dir.z < 0.0f ? -1 : 0);

        float tMaxX = nextBoundary(origin.x, dir.x, stepX, blockX);
        float tMaxY = nextBoundary(origin.y, dir.y, stepY, blockY);
        float tMaxZ = nextBoundary(origin.z, dir.z, stepZ, blockZ);

        float tDeltaX = stepX == 0 ? Float.POSITIVE_INFINITY : Math.abs(1.0f / dir.x);
        float tDeltaY = stepY == 0 ? Float.POSITIVE_INFINITY : Math.abs(1.0f / dir.y);
        float tDeltaZ = stepZ == 0 ? Float.POSITIVE_INFINITY : Math.abs(1.0f / dir.z);

        float distance = 0.0f;

        while (distance <= maxDistance) {
            int id = world.getBlock(blockX, blockY, blockZ);
            BlockType block = BlockType.getById(id);
            if (filter.test(block)) {
                Vector3f hitPos = new Vector3f(origin).fma(distance, dir);
                int faceX = 0;
                int faceY = 0;
                int faceZ = 0;

                if (tMaxX < tMaxY && tMaxX < tMaxZ) {
                    faceX = -stepX;
                } else if (tMaxY < tMaxZ) {
                    faceY = -stepY;
                } else {
                    faceZ = -stepZ;
                }

                return new RaycastResult(true, blockX, blockY, blockZ, faceX, faceY, faceZ, distance, hitPos, block);
            }

            if (tMaxX < tMaxY) {
                if (tMaxX < tMaxZ) {
                    blockX += stepX;
                    distance = tMaxX;
                    tMaxX += tDeltaX;
                } else {
                    blockZ += stepZ;
                    distance = tMaxZ;
                    tMaxZ += tDeltaZ;
                }
            } else {
                if (tMaxY < tMaxZ) {
                    blockY += stepY;
                    distance = tMaxY;
                    tMaxY += tDeltaY;
                } else {
                    blockZ += stepZ;
                    distance = tMaxZ;
                    tMaxZ += tDeltaZ;
                }
            }
        }

        return RaycastResult.MISS;
    }

    private static float nextBoundary(float origin, float dir, int step, int currentBlock) {
        if (step == 0) {
            return Float.POSITIVE_INFINITY;
        }
        float boundary = currentBlock + (step > 0 ? 1.0f : 0.0f);
        float t = (boundary - origin) / dir;
        return Math.max(0.0f, t);
    }
}
