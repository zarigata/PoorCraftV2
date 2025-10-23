package com.poorcraft.common.entity.component;

import com.poorcraft.common.entity.Component;

public class HealthComponent implements Component {
    private static final float DEFAULT_MAX_HEALTH = 20.0f;
    private static final float DEFAULT_MAX_HUNGER = 20.0f;
    private static final float REGENERATION_INTERVAL = 1.0f;
    private static final float HUNGER_REGEN_THRESHOLD = 18.0f;
    private static final float HUNGER_EXHAUSTION_RATE = 0.1f;

    private float health;
    private float maxHealth;
    private float hunger;
    private float maxHunger;
    private float saturation;
    private float regenerationTimer;

    public HealthComponent() {
        this.maxHealth = DEFAULT_MAX_HEALTH;
        this.health = DEFAULT_MAX_HEALTH;
        this.maxHunger = DEFAULT_MAX_HUNGER;
        this.hunger = DEFAULT_MAX_HUNGER;
        this.saturation = DEFAULT_MAX_HUNGER;
        this.regenerationTimer = 0.0f;
    }

    public float getHealth() {
        return health;
    }

    public void setHealth(float health) {
        this.health = clamp(health, 0.0f, maxHealth);
    }

    public float getMaxHealth() {
        return maxHealth;
    }

    public void setMaxHealth(float maxHealth) {
        this.maxHealth = Math.max(0.0f, maxHealth);
        this.health = clamp(this.health, 0.0f, this.maxHealth);
    }

    public void damage(float amount) {
        if (amount <= 0.0f) {
            return;
        }
        setHealth(health - amount);
    }

    public void heal(float amount) {
        if (amount <= 0.0f) {
            return;
        }
        setHealth(health + amount);
    }

    public boolean isDead() {
        return health <= 0.0f;
    }

    public float getHunger() {
        return hunger;
    }

    public void setHunger(float hunger) {
        this.hunger = clamp(hunger, 0.0f, maxHunger);
    }

    public float getMaxHunger() {
        return maxHunger;
    }

    public void setMaxHunger(float maxHunger) {
        this.maxHunger = Math.max(0.0f, maxHunger);
        this.hunger = clamp(this.hunger, 0.0f, this.maxHunger);
    }

    public void addHunger(float amount) {
        if (amount <= 0.0f) {
            return;
        }
        setHunger(hunger + amount);
    }

    public void removeHunger(float amount) {
        if (amount <= 0.0f) {
            return;
        }
        setHunger(hunger - amount);
    }

    public float getSaturation() {
        return saturation;
    }

    public void setSaturation(float saturation) {
        this.saturation = Math.max(0.0f, saturation);
    }

    public void update(float deltaTime) {
        if (isDead()) {
            return;
        }

        if (hunger >= HUNGER_REGEN_THRESHOLD && saturation > 0.0f) {
            regenerationTimer += deltaTime;
            if (regenerationTimer >= REGENERATION_INTERVAL) {
                heal(1.0f);
                regenerationTimer -= REGENERATION_INTERVAL;
                saturation = Math.max(0.0f, saturation - HUNGER_EXHAUSTION_RATE);
            }
        } else {
            regenerationTimer = 0.0f;
            if (hunger <= 0.0f) {
                damage(1.0f * deltaTime);
            }
        }
    }

    private static float clamp(float value, float min, float max) {
        return Math.max(min, Math.min(max, value));
    }
}
