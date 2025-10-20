package com.poorcraft.common.entity.component;

import com.poorcraft.common.entity.Component;
import com.poorcraft.common.entity.animation.AnimationClip;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

/**
 * Tracks animation state for an entity.
 */
public class AnimationComponent implements Component {
    private String currentAnimation;
    private float animationTime;
    private float animationSpeed;
    private boolean looping;
    private final Map<String, AnimationClip> animations;

    public AnimationComponent() {
        this.currentAnimation = "idle";
        this.animationTime = 0.0f;
        this.animationSpeed = 1.0f;
        this.looping = true;
        this.animations = new HashMap<>();
    }

    public void setAnimation(String name, boolean loop) {
        if (name == null || name.isEmpty()) {
            return;
        }
        this.currentAnimation = name;
        this.looping = loop;
        resetAnimation();
    }

    public String getCurrentAnimation() {
        return currentAnimation;
    }

    public float getAnimationTime() {
        return animationTime;
    }

    public void setAnimationTime(float animationTime) {
        this.animationTime = animationTime;
    }

    public void resetAnimation() {
        this.animationTime = 0.0f;
    }

    public void updateTime(float dt) {
        this.animationTime += dt * animationSpeed;
        float duration = getCurrentClipDuration();
        if (looping && duration > 0.0f) {
            this.animationTime = this.animationTime % duration;
        } else if (!looping && duration > 0.0f && this.animationTime > duration) {
            this.animationTime = duration;
        }
    }

    public boolean isLooping() {
        return looping;
    }

    public void setLooping(boolean looping) {
        this.looping = looping;
    }

    public float getAnimationSpeed() {
        return animationSpeed;
    }

    public void setAnimationSpeed(float animationSpeed) {
        this.animationSpeed = animationSpeed;
    }

    public boolean isAnimationFinished() {
        float duration = getCurrentClipDuration();
        return !looping && duration > 0.0f && animationTime >= duration;
    }

    public Map<String, AnimationClip> getAnimations() {
        return Collections.unmodifiableMap(animations);
    }

    public void addAnimation(String name, AnimationClip clip) {
        if (name != null && clip != null) {
            animations.put(name, clip);
        }
    }

    public AnimationClip getAnimation(String name) {
        return name == null ? null : animations.get(name);
    }

    private float getCurrentClipDuration() {
        AnimationClip clip = animations.get(currentAnimation);
        return clip == null ? 0.0f : clip.getDuration();
    }
}
