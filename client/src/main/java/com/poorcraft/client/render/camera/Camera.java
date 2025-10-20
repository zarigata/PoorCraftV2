package com.poorcraft.client.render.camera;

import com.poorcraft.common.Constants;
import org.joml.Matrix4f;
import org.joml.Quaternionf;
import org.joml.Vector3f;

/**
 * 3D perspective camera using JOML for matrix math.
 */
public class Camera {
    private final Vector3f position;
    private final Quaternionf rotation;
    
    private float yaw;
    private float pitch;
    
    private float fov;
    private float aspectRatio;
    private float nearPlane;
    private float farPlane;
    private boolean useReverseZ;
    
    private final Matrix4f viewMatrix;
    private final Matrix4f projectionMatrix;
    private final Matrix4f viewProjectionMatrix;
    
    private boolean viewDirty;
    private boolean projectionDirty;
    
    /**
     * Creates a new camera with default settings.
     */
    public Camera() {
        this.position = new Vector3f(0, 0, 0);
        this.rotation = new Quaternionf();
        this.yaw = 0.0f;
        this.pitch = 0.0f;
        
        this.fov = Constants.Rendering.DEFAULT_FOV;
        this.aspectRatio = 16.0f / 9.0f;
        this.nearPlane = Constants.Rendering.DEFAULT_NEAR_PLANE;
        this.farPlane = Constants.Rendering.DEFAULT_FAR_PLANE;
        this.useReverseZ = false;
        
        this.viewMatrix = new Matrix4f();
        this.projectionMatrix = new Matrix4f();
        this.viewProjectionMatrix = new Matrix4f();
        
        this.viewDirty = true;
        this.projectionDirty = true;
    }
    
    /**
     * Sets the camera position.
     * 
     * @param position The new position
     */
    public void setPosition(Vector3f position) {
        this.position.set(position);
        viewDirty = true;
    }
    
    /**
     * Sets the camera position.
     * 
     * @param x X coordinate
     * @param y Y coordinate
     * @param z Z coordinate
     */
    public void setPosition(float x, float y, float z) {
        this.position.set(x, y, z);
        viewDirty = true;
    }
    
    /**
     * Sets the camera rotation using yaw and pitch angles.
     * 
     * @param yaw Yaw angle in degrees
     * @param pitch Pitch angle in degrees
     */
    public void setRotation(float yaw, float pitch) {
        this.yaw = yaw;
        this.pitch = Math.max(-89.0f, Math.min(89.0f, pitch)); // Clamp pitch
        
        // Convert to quaternion
        rotation.identity()
            .rotateY((float) Math.toRadians(-yaw))
            .rotateX((float) Math.toRadians(-pitch));
        
        viewDirty = true;
    }
    
    /**
     * Sets the perspective projection parameters.
     * 
     * @param fov Field of view in degrees
     * @param aspectRatio Aspect ratio (width / height)
     * @param nearPlane Near clipping plane
     * @param farPlane Far clipping plane
     */
    public void setPerspective(float fov, float aspectRatio, float nearPlane, float farPlane) {
        this.fov = fov;
        this.aspectRatio = aspectRatio;
        this.nearPlane = nearPlane;
        this.farPlane = farPlane;
        projectionDirty = true;
    }
    
    /**
     * Enables or disables reverse-Z projection.
     * Reverse-Z provides better depth precision for large view distances.
     * Requires GL_ARB_clip_control support (OpenGL 4.5+).
     * 
     * @param useReverseZ Whether to use reverse-Z projection
     */
    public void setReverseZ(boolean useReverseZ) {
        if (this.useReverseZ != useReverseZ) {
            this.useReverseZ = useReverseZ;
            projectionDirty = true;
        }
    }
    
    /**
     * Sets up a look-at view matrix.
     * 
     * @param eye Camera position
     * @param target Target position to look at
     * @param up Up vector
     */
    public void lookAt(Vector3f eye, Vector3f target, Vector3f up) {
        this.position.set(eye);
        viewMatrix.setLookAt(eye, target, up);
        viewDirty = false;
    }
    
    /**
     * Moves the camera by a delta vector.
     * 
     * @param delta Movement delta
     */
    public void move(Vector3f delta) {
        position.add(delta);
        viewDirty = true;
    }
    
    /**
     * Rotates the camera by delta angles.
     * 
     * @param dyaw Yaw delta in degrees
     * @param dpitch Pitch delta in degrees
     */
    public void rotate(float dyaw, float dpitch) {
        setRotation(yaw + dyaw, pitch + dpitch);
    }
    
    /**
     * Updates the view matrix if dirty.
     */
    public void updateViewMatrix() {
        if (!viewDirty) return;
        
        // Create rotation matrix from quaternion
        Matrix4f rotationMatrix = new Matrix4f().set(rotation);
        
        // Create translation matrix
        Matrix4f translationMatrix = new Matrix4f().translation(-position.x, -position.y, -position.z);
        
        // Combine: view = rotation * translation
        viewMatrix.set(rotationMatrix).mul(translationMatrix);
        
        viewDirty = false;
    }
    
    /**
     * Updates the projection matrix if dirty.
     */
    public void updateProjectionMatrix() {
        if (!projectionDirty) return;
        
        float fovRadians = (float) Math.toRadians(fov);
        
        if (useReverseZ) {
            // Reverse-Z: swap near and far, use infinite far plane
            projectionMatrix.setPerspective(fovRadians, aspectRatio, farPlane, nearPlane);
        } else {
            // Standard projection
            projectionMatrix.setPerspective(fovRadians, aspectRatio, nearPlane, farPlane);
        }
        
        projectionDirty = false;
    }
    
    /**
     * Updates the combined view-projection matrix.
     */
    public void updateViewProjectionMatrix() {
        updateViewMatrix();
        updateProjectionMatrix();
        projectionMatrix.mul(viewMatrix, viewProjectionMatrix);
    }
    
    /**
     * Updates all matrices.
     */
    public void update() {
        updateViewProjectionMatrix();
    }
    
    /**
     * Gets the forward direction vector.
     * 
     * @return Forward vector
     */
    public Vector3f getForward() {
        return new Vector3f(0, 0, -1).rotate(rotation);
    }
    
    /**
     * Gets the right direction vector.
     * 
     * @return Right vector
     */
    public Vector3f getRight() {
        return new Vector3f(1, 0, 0).rotate(rotation);
    }
    
    /**
     * Gets the up direction vector.
     * 
     * @return Up vector
     */
    public Vector3f getUp() {
        return new Vector3f(0, 1, 0).rotate(rotation);
    }
    
    // Getters
    
    public Vector3f getPosition() {
        return new Vector3f(position);
    }
    
    public float getYaw() {
        return yaw;
    }
    
    public float getPitch() {
        return pitch;
    }
    
    public Matrix4f getViewMatrix() {
        updateViewMatrix();
        return viewMatrix;
    }
    
    public Matrix4f getProjectionMatrix() {
        updateProjectionMatrix();
        return projectionMatrix;
    }
    
    public Matrix4f getViewProjectionMatrix() {
        updateViewProjectionMatrix();
        return viewProjectionMatrix;
    }
    
    public float getFov() {
        return fov;
    }
    
    public float getAspectRatio() {
        return aspectRatio;
    }
    
    public float getNearPlane() {
        return nearPlane;
    }
    
    public float getFarPlane() {
        return farPlane;
    }
}
