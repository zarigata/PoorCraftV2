package com.poorcraft.common.entity.animation;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class AnimationClip {
    private final String name;
    private float duration;
    private boolean looping;
    private final Map<String, List<AnimationKeyframe>> keyframes;

    public AnimationClip(String name, float duration) {
        this.name = name;
        this.duration = duration;
        this.looping = true;
        this.keyframes = new HashMap<>();
    }

    public String getName() {
        return name;
    }

    public float getDuration() {
        return duration;
    }

    public void setDuration(float duration) {
        this.duration = duration;
    }

    public boolean isLooping() {
        return looping;
    }

    public void setLooping(boolean looping) {
        this.looping = looping;
    }

    public void addKeyframe(String boneName, AnimationKeyframe keyframe) {
        keyframes.computeIfAbsent(boneName, ignored -> new ArrayList<>()).add(keyframe);
        keyframes.get(boneName).sort(Comparator.comparing(AnimationKeyframe::getTime));
    }

    public List<AnimationKeyframe> getKeyframes(String boneName) {
        List<AnimationKeyframe> list = keyframes.get(boneName);
        if (list == null) {
            return Collections.emptyList();
        }
        return Collections.unmodifiableList(list);
    }

    public AnimationKeyframe sampleAnimation(String boneName, float time) {
        List<AnimationKeyframe> list = keyframes.get(boneName);
        if (list == null || list.isEmpty()) {
            return null;
        }
        if (list.size() == 1) {
            return list.get(0);
        }
        float wrapped = looping && duration > 0.0f ? time % duration : Math.min(time, duration);
        AnimationKeyframe previous = list.get(0);
        for (int i = 1; i < list.size(); i++) {
            AnimationKeyframe next = list.get(i);
            if (wrapped <= next.getTime()) {
                float span = next.getTime() - previous.getTime();
                float factor = span <= 0.0f ? 0.0f : (wrapped - previous.getTime()) / span;
                return AnimationKeyframe.interpolate(previous, next, factor);
            }
            previous = next;
        }
        return list.get(list.size() - 1);
    }
}
