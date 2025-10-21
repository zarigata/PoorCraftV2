package com.poorcraft.common.entity.component;

import com.poorcraft.common.entity.Component;

public class PhysicsComponent implements Component {

    private boolean affectedByGravity;
    private boolean hasCollision;
    private float mass;
    private boolean canJump;
    private float jumpCooldown;
    private boolean sprinting;
    private boolean crouching;

    public PhysicsComponent() {
        this.affectedByGravity = true;
        this.hasCollision = true;
        this.mass = 1.0f;
        this.canJump = true;
        this.jumpCooldown = 0.0f;
        this.sprinting = false;
        this.crouching = false;
    }

    public boolean isAffectedByGravity() {
        return affectedByGravity;
    }

    public void setAffectedByGravity(boolean affectedByGravity) {
        this.affectedByGravity = affectedByGravity;
    }

    public boolean hasCollision() {
        return hasCollision;
    }

    public void setHasCollision(boolean hasCollision) {
        this.hasCollision = hasCollision;
    }

    public float getMass() {
        return mass;
    }

    public void setMass(float mass) {
        this.mass = mass;
    }

    public boolean canJump() {
        return canJump;
    }

    public void setCanJump(boolean canJump) {
        this.canJump = canJump;
    }

    public float getJumpCooldown() {
        return jumpCooldown;
    }

    public void setJumpCooldown(float jumpCooldown) {
        this.jumpCooldown = jumpCooldown;
    }

    public boolean isSprinting() {
        return sprinting;
    }

    public void setSprinting(boolean sprinting) {
        this.sprinting = sprinting;
    }

    public boolean isCrouching() {
        return crouching;
    }

    public void setCrouching(boolean crouching) {
        this.crouching = crouching;
    }

    public boolean canJumpNow() {
        return canJump && jumpCooldown <= 0.0f;
    }

    public void resetJumpCooldown(float cooldown) {
        this.jumpCooldown = cooldown;
    }

    public void updateJumpCooldown(float dt) {
        if (jumpCooldown > 0.0f) {
            jumpCooldown = Math.max(0.0f, jumpCooldown - dt);
        }
    }
}
