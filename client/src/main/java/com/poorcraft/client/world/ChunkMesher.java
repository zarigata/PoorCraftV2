package com.poorcraft.client.world;

import com.poorcraft.common.world.block.BlockType;
import com.poorcraft.common.world.chunk.Chunk;

import java.util.function.Function;

/**
 * Implements greedy meshing algorithm for chunk rendering.
 */
public class ChunkMesher {
    
    private final Chunk chunk;
    private final Chunk[] neighbors; // [North, South, East, West]
    private final Function<String, Integer> textureLayerLookup;
    
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
     * @param textureLayerLookup Function to lookup texture layer by name
     */
    public ChunkMesher(Chunk chunk, Chunk[] neighbors, Function<String, Integer> textureLayerLookup) {
        this.chunk = chunk;
        this.neighbors = neighbors;
        this.textureLayerLookup = textureLayerLookup;
    }
    
    /**
     * Generates mesh for the chunk using greedy meshing.
     * 
     * @return ChunkMesh
     */
    public ChunkMesh mesh() {
        ChunkMesh mesh = new ChunkMesh();
        
        // Chunk dimensions: X=16, Y=256, Z=16
        int[] dims = {16, 256, 16};
        
        // Process each axis
        for (int axis = 0; axis < 3; axis++) {
            int u = (axis + 1) % 3;
            int v = (axis + 2) % 3;
            
            int[] x = new int[3];
            int[] q = new int[3];
            
            // Compute mask dimensions for this axis
            int uDim = dims[u];
            int vDim = dims[v];
            int maskSize = uDim * vDim;
            
            boolean[] mask = new boolean[maskSize];
            BlockType[] maskBlocks = new BlockType[maskSize];
            
            q[axis] = 1;
            
            // Process both directions along this axis
            for (int direction = -1; direction <= 1; direction += 2) {
                for (x[axis] = -1; x[axis] < dims[axis];) {
                    // Build mask
                    int n = 0;
                    for (x[v] = 0; x[v] < vDim; x[v]++) {
                        for (x[u] = 0; x[u] < uDim; x[u]++) {
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
                    for (int j = 0; j < vDim; j++) {
                        for (int i = 0; i < uDim;) {
                            if (mask[n]) {
                                BlockType block = maskBlocks[n];
                                
                                // Compute width
                                int w;
                                for (w = 1; i + w < uDim && mask[n + w] && maskBlocks[n + w] == block; w++) {}
                                
                                // Compute height
                                boolean done = false;
                                int h;
                                for (h = 1; j + h < vDim; h++) {
                                    for (int k = 0; k < w; k++) {
                                        if (!mask[n + k + h * uDim] || maskBlocks[n + k + h * uDim] != block) {
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
                                        mask[n + k + l * uDim] = false;
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
        
        // Calculate quad vertices with UV tiling per block
        float uSpan = Math.abs(du[0]) + Math.abs(du[1]) + Math.abs(du[2]);
        float vSpan = Math.abs(dv[0]) + Math.abs(dv[1]) + Math.abs(dv[2]);
        float[] v0 = {x, y, z, 0.0f, 0.0f, normal[0], normal[1], normal[2], texLayer, ao};
        float[] v1Arr = {x + du[0], y + du[1], z + du[2], uSpan, 0.0f, normal[0], normal[1], normal[2], texLayer, ao};
        float[] v2Arr = {x + du[0] + dv[0], y + du[1] + dv[1], z + du[2] + dv[2], uSpan, vSpan, normal[0], normal[1], normal[2], texLayer, ao};
        float[] v3Arr = {x + dv[0], y + dv[1], z + dv[2], 0.0f, vSpan, normal[0], normal[1], normal[2], texLayer, ao};
        
        if (direction > 0) {
            mesh.addQuad(v0, v1Arr, v2Arr, v3Arr);
        } else {
            mesh.addQuad(v3Arr, v2Arr, v1Arr, v0);
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
        // Get face-specific texture name from block
        String textureName = block.getTextureName(face);
        // Lookup texture layer index
        return textureLayerLookup.apply(textureName);
    }
}
