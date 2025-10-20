package com.poorcraft.common.entity.animation;

import java.util.HashMap;
import java.util.Map;

public class AnimationController {
    private final Map<String, AnimationClip> clips;
    private AnimationClip currentClip;
    private float currentTime;
    private boolean playing;
    private float speed;
    private float blendTime;

    public AnimationController() {
        this.clips = new HashMap<>();
        this.currentClip = null;
        this.currentTime = 0.0f;
        this.playing = false;
        this.speed = 1.0f;
        this.blendTime = 0.0f;
    }

    public void addClip(AnimationClip clip) {
        if (clip != null) {
            clips.put(clip.getName(), clip);
        }
    }

    public void playClip(String name, boolean loop) {
        AnimationClip clip = clips.get(name);
        if (clip == null) {
            return;
        }
        currentClip = clip;
        currentClip.setLooping(loop);
        currentTime = 0.0f;
        playing = true;
    }

    public void stopClip() {
        playing = false;
    }

    public void update(float dt) {
        if (!playing || currentClip == null) {
            return;
        }
        currentTime += dt * speed;
        float duration = currentClip.getDuration();
        if (currentClip.isLooping() && duration > 0.0f) {
            currentTime %= duration;
        } else if (!currentClip.isLooping() && currentTime > duration) {
            currentTime = duration;
            playing = false;
        }
    }

    public AnimationKeyframe getBonePose(String boneName) {
        if (currentClip == null) {
            return null;
        }
        return currentClip.sampleAnimation(boneName, currentTime);
    }

    public AnimationClip getCurrentClip() {
        return currentClip;
    }

    public float getCurrentTime() {
        return currentTime;
    }

    public boolean isPlaying() {
        return playing;
    }

    public void setSpeed(float speed) {
        this.speed = speed;
    }

    public float getSpeed() {
        return speed;
    }

    public void setBlendTime(float blendTime) {
        this.blendTime = blendTime;
    }

    public float getBlendTime() {
        return blendTime;
    }
}
