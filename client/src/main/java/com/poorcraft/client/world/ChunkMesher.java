package com.poorcraft.client.world;

import com.poorcraft.common.world.block.BlockType;
import com.poorcraft.common.world.chunk.Chunk;

/**
 * Implements greedy meshing algorithm for chunk rendering.
 */
public class ChunkMesher {
    
    private final Chunk chunk;
    private final Chunk[] neighbors; // [North, South, East, West]
    
    // Face directions: 0=bottom, 1=top, 2=north, 3=south, 4=west, 5=east
    private static final int[][] FACE_NORMALS = {
        {0, -1, 0},  // Bottom
        {0, 1, 0},   // Top
        {0, 0, -1},  // North
        {0, 0, 1},   // South
        {-1, 0, 0},  // West
        {1, 0, 0}    // East
    };
    
    /**
     * Creates a chunk mesher.
     * 
     * @param chunk Chunk to mesh
     * @param neighbors Neighbor chunks [N, S, E, W] for seamless meshing
     */
    public ChunkMesher(Chunk chunk, Chunk[] neighbors) {
        this.chunk = chunk;
        this.neighbors = neighbors;
    }
    
    /**
     * Generates mesh for the chunk using greedy meshing.
     * 
     * @return ChunkMesh
     */
    public ChunkMesh mesh() {
        ChunkMesh mesh = new ChunkMesh();
        
        // Process each axis
        for (int axis = 0; axis < 3; axis++) {
            int u = (axis + 1) % 3;
            int v = (axis + 2) % 3;
            
            int[] x = new int[3];
            int[] q = new int[3];
            
            boolean[] mask = new boolean[16 * 256];
            BlockType[] maskBlocks = new BlockType[16 * 256];
            
            q[axis] = 1;
            
            // Process both directions along this axis
            for (int direction = -1; direction <= 1; direction += 2) {
                for (x[axis] = -1; x[axis] < 16;) {
                    // Build mask
                    int n = 0;
                    for (x[v] = 0; x[v] < 16; x[v]++) {
                        for (x[u] = 0; x[u] < 256; x[u]++) {
                            BlockType blockCurrent = getBlock(x[0], x[1], x[2]);
                            BlockType blockCompare = getBlock(x[0] + q[0] * direction, 
                                                             x[1] + q[1] * direction, 
                                                             x[2] + q[2] * direction);
                            
                            boolean visible = isBlockVisible(blockCurrent, blockCompare, direction > 0);
                            mask[n] = visible;
                            maskBlocks[n] = visible ? blockCurrent : null;
                            n++;
                        }
                    }
                    
                    x[axis]++;
                    
                    // Generate mesh from mask using greedy algorithm
                    n = 0;
                    for (int j = 0; j < 16; j++) {
                        for (int i = 0; i < 256;) {
                            if (mask[n]) {
                                BlockType block = maskBlocks[n];
                                
                                // Compute width
                                int w;
                                for (w = 1; i + w < 256 && mask[n + w] && maskBlocks[n + w] == block; w++) {}
                                
                                // Compute height
                                boolean done = false;
                                int h;
                                for (h = 1; j + h < 16; h++) {
                                    for (int k = 0; k < w; k++) {
                                        if (!mask[n + k + h * 256] || maskBlocks[n + k + h * 256] != block) {
                                            done = true;
                                            break;
                                        }
                                    }
                                    if (done) break;
                                }
                                
                                // Add quad
                                x[u] = i;
                                x[v] = j;
                                
                                int[] du = new int[3];
                                int[] dv = new int[3];
                                du[u] = w;
                                dv[v] = h;
                                
                                int faceIndex = axis * 2 + (direction > 0 ? 1 : 0);
                                addQuad(mesh, x, du, dv, direction, faceIndex, block);
                                
                                // Clear mask
                                for (int l = 0; l < h; l++) {
                                    for (int k = 0; k < w; k++) {
                                        mask[n + k + l * 256] = false;
                                    }
                                }
                                
                                i += w;
                                n += w;
                            } else {
                                i++;
                                n++;
                            }
                        }
                    }
                }
            }
        }
        
        return mesh;
    }
    
    private void addQuad(ChunkMesh mesh, int[] pos, int[] du, int[] dv, int direction, 
                        int faceIndex, BlockType block) {
        float x = pos[0];
        float y = pos[1];
        float z = pos[2];
        
        float[] normal = {FACE_NORMALS[faceIndex][0], 
                         FACE_NORMALS[faceIndex][1], 
                         FACE_NORMALS[faceIndex][2]};
        
        // Get texture layer (simplified - would need texture atlas mapping)
        float texLayer = getTextureLayer(block, faceIndex);
        float ao = 1.0f; // Ambient occlusion (future enhancement)
        
        // Calculate quad vertices
        float[] v0 = {x, y, z, 0, 0, normal[0], normal[1], normal[2], texLayer, ao};
        float[] v1 = {x + du[0], y + du[1], z + du[2], 1, 0, normal[0], normal[1], normal[2], texLayer, ao};
        float[] v2 = {x + du[0] + dv[0], y + du[1] + dv[1], z + du[2] + dv[2], 1, 1, normal[0], normal[1], normal[2], texLayer, ao};
        float[] v3 = {x + dv[0], y + dv[1], z + dv[2], 0, 1, normal[0], normal[1], normal[2], texLayer, ao};
        
        if (direction > 0) {
            mesh.addQuad(v0, v1, v2, v3);
        } else {
            mesh.addQuad(v3, v2, v1, v0);
        }
    }
    
    private boolean isBlockVisible(BlockType current, BlockType neighbor, boolean positive) {
        if (current.isAir()) {
            return false;
        }
        
        if (neighbor.isAir()) {
            return true;
        }
        
        if (neighbor.isTransparent() && !current.isTransparent()) {
            return true;
        }
        
        return false;
    }
    
    private BlockType getBlock(int x, int y, int z) {
        // Handle out of bounds
        if (y < 0 || y >= 256) {
            return BlockType.AIR;
        }
        
        // Check if in current chunk
        if (x >= 0 && x < 16 && z >= 0 && z < 16) {
            return chunk.getBlockType(x, y, z);
        }
        
        // Check neighbors
        if (neighbors != null) {
            if (z < 0 && neighbors[0] != null) {
                return neighbors[0].getBlockType(x, y, 15);
            }
            if (z >= 16 && neighbors[1] != null) {
                return neighbors[1].getBlockType(x, y, 0);
            }
            if (x >= 16 && neighbors[2] != null) {
                return neighbors[2].getBlockType(0, y, z);
            }
            if (x < 0 && neighbors[3] != null) {
                return neighbors[3].getBlockType(15, y, z);
            }
        }
        
        return BlockType.AIR;
    }
    
    private float getTextureLayer(BlockType block, int face) {
        // Simplified texture mapping - returns block ID as layer
        // In a real implementation, this would map to texture atlas indices
        return block.getId();
    }
}
