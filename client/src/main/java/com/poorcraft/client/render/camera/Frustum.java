package com.poorcraft.client.render.camera;

import org.joml.Matrix4f;
import org.joml.Vector3f;
import org.joml.Vector4f;

/**
 * View frustum for culling using the Gribb-Hartmann method.
 */
public class Frustum {
    private static final int LEFT = 0;
    private static final int RIGHT = 1;
    private static final int BOTTOM = 2;
    private static final int TOP = 3;
    private static final int NEAR = 4;
    private static final int FAR = 5;
    
    private final Vector4f[] planes;
    
    /**
     * Creates a new frustum.
     */
    public Frustum() {
        this.planes = new Vector4f[6];
        for (int i = 0; i < 6; i++) {
            planes[i] = new Vector4f();
        }
    }
    
    /**
     * Extracts frustum planes from a view-projection matrix.
     * Uses the Gribb-Hartmann method.
     * 
     * @param viewProjection The combined view-projection matrix
     */
    public void extractPlanes(Matrix4f viewProjection) {
        // Extract rows from matrix
        float m00 = viewProjection.m00(), m01 = viewProjection.m01(), m02 = viewProjection.m02(), m03 = viewProjection.m03();
        float m10 = viewProjection.m10(), m11 = viewProjection.m11(), m12 = viewProjection.m12(), m13 = viewProjection.m13();
        float m20 = viewProjection.m20(), m21 = viewProjection.m21(), m22 = viewProjection.m22(), m23 = viewProjection.m23();
        float m30 = viewProjection.m30(), m31 = viewProjection.m31(), m32 = viewProjection.m32(), m33 = viewProjection.m33();
        
        // Left plane: row3 + row0
        planes[LEFT].set(m30 + m00, m31 + m01, m32 + m02, m33 + m03);
        normalizePlane(planes[LEFT]);
        
        // Right plane: row3 - row0
        planes[RIGHT].set(m30 - m00, m31 - m01, m32 - m02, m33 - m03);
        normalizePlane(planes[RIGHT]);
        
        // Bottom plane: row3 + row1
        planes[BOTTOM].set(m30 + m10, m31 + m11, m32 + m12, m33 + m13);
        normalizePlane(planes[BOTTOM]);
        
        // Top plane: row3 - row1
        planes[TOP].set(m30 - m10, m31 - m11, m32 - m12, m33 - m13);
        normalizePlane(planes[TOP]);
        
        // Near plane: row3 + row2
        planes[NEAR].set(m30 + m20, m31 + m21, m32 + m22, m33 + m23);
        normalizePlane(planes[NEAR]);
        
        // Far plane: row3 - row2
        planes[FAR].set(m30 - m20, m31 - m21, m32 - m22, m33 - m23);
        normalizePlane(planes[FAR]);
    }
    
    /**
     * Normalizes a plane equation.
     */
    private void normalizePlane(Vector4f plane) {
        float length = (float) Math.sqrt(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
        if (length > 0.0f) {
            plane.div(length);
        }
    }
    
    /**
     * Tests if a sphere intersects or is inside the frustum.
     * 
     * @param center Sphere center
     * @param radius Sphere radius
     * @return true if the sphere is visible
     */
    public boolean testSphere(Vector3f center, float radius) {
        for (Vector4f plane : planes) {
            float distance = plane.x * center.x + plane.y * center.y + plane.z * center.z + plane.w;
            if (distance < -radius) {
                return false; // Sphere is completely outside this plane
            }
        }
        return true; // Sphere intersects or is inside frustum
    }
    
    /**
     * Tests if an axis-aligned bounding box intersects or is inside the frustum.
     * 
     * @param min AABB minimum point
     * @param max AABB maximum point
     * @return true if the AABB is visible
     */
    public boolean testAABB(Vector3f min, Vector3f max) {
        for (Vector4f plane : planes) {
            // Get positive vertex (farthest point in plane normal direction)
            Vector3f pVertex = new Vector3f(
                plane.x > 0 ? max.x : min.x,
                plane.y > 0 ? max.y : min.y,
                plane.z > 0 ? max.z : min.z
            );
            
            // Test positive vertex
            float distance = plane.x * pVertex.x + plane.y * pVertex.y + plane.z * pVertex.z + plane.w;
            if (distance < 0) {
                return false; // AABB is completely outside this plane
            }
        }
        return true; // AABB intersects or is inside frustum
    }
    
    /**
     * Tests if a point is inside the frustum.
     * 
     * @param point The point to test
     * @return true if the point is inside
     */
    public boolean testPoint(Vector3f point) {
        for (Vector4f plane : planes) {
            float distance = plane.x * point.x + plane.y * point.y + plane.z * point.z + plane.w;
            if (distance < 0) {
                return false;
            }
        }
        return true;
    }
}
