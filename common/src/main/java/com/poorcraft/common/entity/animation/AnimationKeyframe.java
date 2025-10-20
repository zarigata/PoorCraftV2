package com.poorcraft.common.entity.animation;

import org.joml.Quaternionf;
import org.joml.Vector3f;

/**
 * Represents a single bone transform at a given time within an animation clip.
 */
public class AnimationKeyframe {
    private final float time;
    private final Vector3f position;
    private final Quaternionf rotation;
    private final Vector3f scale;

    public AnimationKeyframe(float time, Vector3f position, Quaternionf rotation, Vector3f scale) {
        this.time = time;
        this.position = new Vector3f(position);
        this.rotation = new Quaternionf(rotation);
        this.scale = new Vector3f(scale);
    }

    public float getTime() {
        return time;
    }

    public Vector3f getPosition() {
        return new Vector3f(position);
    }

    public Quaternionf getRotation() {
        return new Quaternionf(rotation);
    }

    public Vector3f getScale() {
        return new Vector3f(scale);
    }

    public static AnimationKeyframe interpolate(AnimationKeyframe current, AnimationKeyframe next, float t) {
        if (current == null) {
            return next;
        }
        if (next == null) {
            return current;
        }
        float clamped = Math.max(0.0f, Math.min(1.0f, t));
        Vector3f position = current.getPosition().lerp(next.getPosition(), clamped);
        Quaternionf rotation = current.getRotation().slerp(next.getRotation(), clamped);
        Vector3f scale = current.getScale().lerp(next.getScale(), clamped);
        float time = current.time + (next.time - current.time) * clamped;
        return new AnimationKeyframe(time, position, rotation, scale);
    }
}
