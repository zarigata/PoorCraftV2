package com.poorcraft.server.world;

import java.util.UUID;

/**
 * Base class for server-side entities.
 * <p>
 * Represents an entity in the server world with position, rotation, and velocity.
 */
public class ServerEntity {
    private int entityId;
    private UUID uuid;
    private int type;

    private double x, y, z;
    private float yaw, pitch;
    private double velocityX, velocityY, velocityZ;
    private boolean onGround = true;

    private double lastX, lastY, lastZ;
    private boolean positionChanged = false;

    /**
     * Creates a new server entity.
     *
     * @param uuid the entity UUID
     * @param type the entity type ID
     * @param x initial X position
     * @param y initial Y position
     * @param z initial Z position
     */
    public ServerEntity(UUID uuid, int type, double x, double y, double z) {
        this.uuid = uuid;
        this.type = type;
        this.x = x;
        this.y = y;
        this.z = z;
        this.lastX = x;
        this.lastY = y;
        this.lastZ = z;
    }

    /**
     * Updates the entity state.
     */
    public void tick() {
        // Apply velocity
        x += velocityX;
        y += velocityY;
        z += velocityZ;

        // Apply gravity if not on ground
        if (!onGround) {
            velocityY -= 0.08; // Simple gravity
        }

        // Check if position changed significantly
        if (Math.abs(x - lastX) > 0.01 || Math.abs(y - lastY) > 0.01 || Math.abs(z - lastZ) > 0.01) {
            positionChanged = true;
            lastX = x;
            lastY = y;
            lastZ = z;
        }
    }

    /**
     * Sets the entity ID.
     *
     * @param entityId the entity ID
     */
    public void setEntityId(int entityId) {
        this.entityId = entityId;
    }

    /**
     * Gets the entity ID.
     *
     * @return the entity ID
     */
    public int getEntityId() {
        return entityId;
    }

    /**
     * Gets the entity UUID.
     *
     * @return the UUID
     */
    public UUID getUuid() {
        return uuid;
    }

    /**
     * Gets the entity type ID.
     *
     * @return the type ID
     */
    public int getType() {
        return type;
    }

    /**
     * Gets the X position.
     *
     * @return the X coordinate
     */
    public double getX() {
        return x;
    }

    /**
     * Gets the Y position.
     *
     * @return the Y coordinate
     */
    public double getY() {
        return y;
    }

    /**
     * Gets the Z position.
     *
     * @return the Z coordinate
     */
    public double getZ() {
        return z;
    }

    /**
     * Gets the yaw rotation.
     *
     * @return the yaw
     */
    public float getYaw() {
        return yaw;
    }

    /**
     * Gets the pitch rotation.
     *
     * @return the pitch
     */
    public float getPitch() {
        return pitch;
    }

    /**
     * Gets the X velocity.
     *
     * @return the velocity X
     */
    public double getVelocityX() {
        return velocityX;
    }

    /**
     * Gets the Y velocity.
     *
     * @return the velocity Y
     */
    public double getVelocityY() {
        return velocityY;
    }

    /**
     * Gets the Z velocity.
     *
     * @return the velocity Z
     */
    public double getVelocityZ() {
        return velocityZ;
    }

    /**
     * Checks if the entity is on ground.
     *
     * @return true if on ground
     */
    public boolean isOnGround() {
        return onGround;
    }

    /**
     * Sets the position and rotation.
     *
     * @param x position X
     * @param y position Y
     * @param z position Z
     * @param yaw rotation yaw
     * @param pitch rotation pitch
     * @param onGround on ground status
     */
    public void setPosition(double x, double y, double z, float yaw, float pitch, boolean onGround) {
        this.x = x;
        this.y = y;
        this.z = z;
        this.yaw = yaw;
        this.pitch = pitch;
        this.onGround = onGround;

        positionChanged = true;
        lastX = x;
        lastY = y;
        lastZ = z;
    }

    /**
     * Sets the velocity.
     *
     * @param velocityX velocity X
     * @param velocityY velocity Y
     * @param velocityZ velocity Z
     */
    public void setVelocity(double velocityX, double velocityY, double velocityZ) {
        this.velocityX = velocityX;
        this.velocityY = velocityY;
        this.velocityZ = velocityZ;
    }

    /**
     * Checks if the position has changed since last tick.
     *
     * @return true if position changed
     */
    public boolean hasPositionChanged() {
        return positionChanged;
    }

    /**
     * Resets the position changed flag.
     */
    public void resetPositionChanged() {
        positionChanged = false;
    }

    /**
     * Gets the delta X since last update.
     *
     * @return the delta X
     */
    public double getDeltaX() {
        return x - lastX;
    }

    /**
     * Gets the delta Y since last update.
     *
     * @return the delta Y
     */
    public double getDeltaY() {
        return y - lastY;
    }

    /**
     * Gets the delta Z since last update.
     *
     * @return the delta Z
     */
    public double getDeltaZ() {
        return z - lastZ;
    }
}
