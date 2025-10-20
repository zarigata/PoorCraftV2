package com.poorcraft.common.entity.component;

import com.poorcraft.common.entity.Component;
import org.joml.Vector3f;

/**
 * Encapsulates velocity and movement data for an entity.
 */
public class VelocityComponent implements Component {
    private final Vector3f velocity;
    private final Vector3f acceleration;
    private float drag;
    private float maxSpeed;

    public VelocityComponent() {
        this.velocity = new Vector3f();
        this.acceleration = new Vector3f();
        this.drag = 0.98f;
        this.maxSpeed = 10.0f;
    }

    public Vector3f getVelocity() {
        return new Vector3f(velocity);
    }

    public void setVelocity(Vector3f value) {
        this.velocity.set(value);
    }

    public void setVelocity(float x, float y, float z) {
        this.velocity.set(x, y, z);
    }

    public void addVelocity(Vector3f delta) {
        this.velocity.add(delta);
    }

    public Vector3f getAcceleration() {
        return new Vector3f(acceleration);
    }

    public void setAcceleration(Vector3f value) {
        this.acceleration.set(value);
    }

    public float getDrag() {
        return drag;
    }

    public void setDrag(float drag) {
        this.drag = drag;
    }

    public float getMaxSpeed() {
        return maxSpeed;
    }

    public void setMaxSpeed(float maxSpeed) {
        this.maxSpeed = maxSpeed;
    }

    public void applyDrag() {
        velocity.mul(drag);
    }

    public void clampSpeed() {
        float length = velocity.length();
        if (length > maxSpeed && length > 0.0f) {
            velocity.mul(maxSpeed / length);
        }
    }

    public boolean isMoving() {
        return velocity.lengthSquared() > 1.0e-4f;
    }
}
