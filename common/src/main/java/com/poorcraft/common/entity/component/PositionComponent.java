package com.poorcraft.common.entity.component;

import com.poorcraft.common.entity.Component;
import org.joml.Quaternionf;
import org.joml.Vector3f;
public class PositionComponent implements Component {
    private final Vector3f position;
    private final Vector3f previousPosition;
    private final Quaternionf rotation;
    private final Quaternionf previousRotation;
    private float yaw;
    private float pitch;
    private boolean onGround;

    public PositionComponent(Vector3f initialPosition) {
        this.position = new Vector3f(initialPosition);
        this.previousPosition = new Vector3f(initialPosition);
        this.rotation = new Quaternionf();
        this.previousRotation = new Quaternionf();
        this.yaw = 0.0f;
        this.pitch = 0.0f;
        this.onGround = false;
    }

    public Vector3f getPosition() {
        return new Vector3f(position);
    }

    public void setPosition(Vector3f newPosition) {
        this.position.set(newPosition);
    }

    public void setPosition(float x, float y, float z) {
        this.position.set(x, y, z);
    }

    public Quaternionf getRotation() {
        return new Quaternionf(rotation);
    }

    public void setRotation(float yaw, float pitch) {
        this.yaw = yaw;
        this.pitch = pitch;
        this.rotation.identity()
                .rotateY((float) Math.toRadians(yaw))
                .rotateX((float) Math.toRadians(pitch));
    }

    public float getYaw() {
        return yaw;
    }

    public float getPitch() {
        return pitch;
    }

    public boolean isOnGround() {
        return onGround;
    }

    public void setOnGround(boolean onGround) {
        this.onGround = onGround;
    }

    public void updatePrevious() {
        this.previousPosition.set(position);
        this.previousRotation.set(rotation);
    }

    public Vector3f getInterpolatedPosition(double alpha) {
        float ax = (float) alpha;
        return new Vector3f(previousPosition).lerp(position, ax);
    }

    public Quaternionf getInterpolatedRotation(double alpha) {
        float ax = (float) alpha;
        return new Quaternionf(previousRotation).slerp(rotation, ax);
    }
}
