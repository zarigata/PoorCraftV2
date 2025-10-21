package com.poorcraft.common.physics;

import java.util.ArrayList;
import java.util.List;

import org.joml.Vector3f;

import com.poorcraft.common.Constants;
import com.poorcraft.common.config.Configuration;
import com.poorcraft.common.entity.Entity;
import com.poorcraft.common.entity.component.PhysicsComponent;
import com.poorcraft.common.entity.component.PositionComponent;
import com.poorcraft.common.entity.component.VelocityComponent;
import com.poorcraft.common.world.World;
import com.poorcraft.common.world.block.BlockType;

public class PhysicsEngine {

    private final Configuration config;
    private final float gravity;
    private final float terminalVelocity;
    private final float groundFriction;
    private final float airFriction;
    private final float stepHeight;
    private final boolean collisionEnabled;

    public PhysicsEngine(Configuration config) {
        this.config = config;
        this.gravity = config.getFloat("physics.gravity", Constants.Physics.GRAVITY);
        this.terminalVelocity = config.getFloat("physics.terminalVelocity", Constants.Physics.TERMINAL_VELOCITY);
        this.groundFriction = config.getFloat("physics.groundFriction", Constants.Physics.GROUND_FRICTION);
        this.airFriction = config.getFloat("physics.airFriction", Constants.Physics.AIR_FRICTION);
        this.stepHeight = config.getFloat("physics.stepHeight", Constants.Physics.STEP_HEIGHT);
        this.collisionEnabled = config.getBoolean("physics.enableCollision", true);
    }

    public void applyGravity(VelocityComponent velocity, boolean onGround, float dt) {
        if (velocity == null) {
            return;
        }
        if (!onGround && gravity != 0.0f) {
            Vector3f vel = velocity.getVelocity();
            float newY = vel.y + gravity * dt;
            vel.y = Math.max(newY, terminalVelocity);
            velocity.setVelocity(vel);
        }
    }

    public void applyFriction(VelocityComponent velocity, boolean onGround) {
        if (velocity == null) {
            return;
        }
        Vector3f vel = velocity.getVelocity();
        float friction = onGround ? groundFriction : airFriction;
        vel.x *= friction;
        vel.z *= friction;
        velocity.setVelocity(vel);
    }

    public void resolveCollisions(Entity entity, World world, float dt) {
        if (!collisionEnabled || world == null) {
            return;
        }
        PositionComponent position = entity.getPosition();
        VelocityComponent velocity = entity.getVelocity();
        PhysicsComponent physics = entity.getComponent(PhysicsComponent.class);

        if (position == null || velocity == null || physics == null || !physics.hasCollision()) {
            return;
        }

        Vector3f pos = position.getPosition();
        Vector3f vel = velocity.getVelocity();

        Vector3f movement = new Vector3f(vel).mul(dt);
        AABB aabb = AABB.fromEntity(pos, Constants.Player.WIDTH, Constants.Player.HEIGHT);

        boolean onGround = false;

        float movedX = moveWithCollision(world, aabb, movement.x, Axis.X, physics);
        pos.x += movedX;
        if (Math.abs(movedX - movement.x) > Constants.Physics.COLLISION_EPSILON) {
            vel.x = 0.0f;
        }

        float movedY = moveWithCollision(world, aabb, movement.y, Axis.Y, physics);
        pos.y += movedY;
        if (Math.abs(movedY - movement.y) > Constants.Physics.COLLISION_EPSILON) {
            vel.y = 0.0f;
            if (movement.y < 0.0f) {
                onGround = true;
            }
        }

        float movedZ = moveWithCollision(world, aabb, movement.z, Axis.Z, physics);
        pos.z += movedZ;
        if (Math.abs(movedZ - movement.z) > Constants.Physics.COLLISION_EPSILON) {
            vel.z = 0.0f;
        }

        boolean grounded = onGround || checkGroundCollision(aabb, world);
        position.setOnGround(grounded);
        position.setPosition(pos);
        velocity.setVelocity(vel);
    }

    private float moveWithCollision(World world, AABB aabb, float desired, Axis axis, PhysicsComponent physics) {
        if (Math.abs(desired) < Constants.Physics.COLLISION_EPSILON) {
            return 0.0f;
        }

        float moved = sweepAxis(world, aabb, desired, axis);

        if (Math.abs(moved - desired) > Constants.Physics.COLLISION_EPSILON && axis != Axis.Y && physics != null && physics.hasCollision()) {
            float stepped = attemptStep(world, aabb, desired, axis);
            if (Math.abs(stepped - moved) > Constants.Physics.COLLISION_EPSILON) {
                moved = stepped;
            }
        }

        offsetAABB(aabb, axis, moved);
        return moved;
    }

    private float sweepAxis(World world, AABB aabb, float amount, Axis axis) {
        float stepSize = 0.05f;
        float moved = 0.0f;
        float sign = Math.signum(amount);
        float remaining = Math.abs(amount);

        while (remaining > 0.0f) {
            float step = Math.min(stepSize, remaining) * sign;
            if (willCollide(world, aabb, moved + step, axis)) {
                break;
            }
            moved += step;
            remaining -= Math.min(stepSize, remaining);
        }

        return moved;
    }

    private float attemptStep(World world, AABB aabb, float amount, Axis axis) {
        if (stepHeight <= 0.0f || Math.abs(amount) < Constants.Physics.COLLISION_EPSILON) {
            return sweepAxis(world, aabb, amount, axis);
        }

        AABB test = new AABB(aabb.getMinX(), aabb.getMinY(), aabb.getMinZ(), aabb.getMaxX(), aabb.getMaxY(), aabb.getMaxZ());

        if (willCollide(world, test, stepHeight, Axis.Y)) {
            return sweepAxis(world, aabb, amount, axis);
        }

        offsetAABB(test, Axis.Y, stepHeight);

        float horizontal = sweepAxis(world, test, amount, axis);
        if (Math.abs(horizontal - amount) > Constants.Physics.COLLISION_EPSILON) {
            return sweepAxis(world, aabb, amount, axis);
        }

        if (willCollide(world, test, -stepHeight, Axis.Y)) {
            return sweepAxis(world, aabb, amount, axis);
        }

        offsetAABB(aabb, Axis.Y, stepHeight);
        offsetAABB(aabb, axis, amount);
        offsetAABB(aabb, Axis.Y, -stepHeight);
        return amount;
    }

    private boolean willCollide(World world, AABB aabb, float offset, Axis axis) {
        if (Math.abs(offset) < Constants.Physics.COLLISION_EPSILON) {
            return false;
        }
        AABB moved = new AABB(aabb.getMinX(), aabb.getMinY(), aabb.getMinZ(), aabb.getMaxX(), aabb.getMaxY(), aabb.getMaxZ());
        offsetAABB(moved, axis, offset);
        for (AABB block : getBlocksInAABB(moved, world)) {
            if (moved.intersects(block)) {
                return true;
            }
        }
        return false;
    }

    private void offsetAABB(AABB aabb, Axis axis, float amount) {
        if (Math.abs(amount) < Constants.Physics.COLLISION_EPSILON) {
            return;
        }
        switch (axis) {
            case X -> {
                aabb.setMinX(aabb.getMinX() + amount);
                aabb.setMaxX(aabb.getMaxX() + amount);
            }
            case Y -> {
                aabb.setMinY(aabb.getMinY() + amount);
                aabb.setMaxY(aabb.getMaxY() + amount);
            }
            case Z -> {
                aabb.setMinZ(aabb.getMinZ() + amount);
                aabb.setMaxZ(aabb.getMaxZ() + amount);
            }
        }
    }

    private boolean checkGroundCollision(AABB aabb, World world) {
        AABB feet = new AABB(aabb.getMinX(), aabb.getMinY() - 0.1f, aabb.getMinZ(), aabb.getMaxX(), aabb.getMinY(), aabb.getMaxZ());
        for (AABB blockBox : getBlocksInAABB(feet, world)) {
            if (feet.intersects(blockBox)) {
                return true;
            }
        }
        return false;
    }

    private List<AABB> getBlocksInAABB(AABB aabb, World world) {
        int minX = (int) Math.floor(aabb.getMinX());
        int minY = (int) Math.floor(aabb.getMinY());
        int minZ = (int) Math.floor(aabb.getMinZ());
        int maxX = (int) Math.floor(aabb.getMaxX());
        int maxY = (int) Math.floor(aabb.getMaxY());
        int maxZ = (int) Math.floor(aabb.getMaxZ());

        List<AABB> blocks = new ArrayList<>();

        for (int x = minX; x <= maxX; x++) {
            for (int y = minY; y <= maxY; y++) {
                for (int z = minZ; z <= maxZ; z++) {
                    BlockType block = BlockType.getById(world.getBlock(x, y, z));
                    if (block != null && block.isSolid()) {
                        blocks.add(new AABB(x, y, z, x + 1.0f, y + 1.0f, z + 1.0f));
                    }
                }
            }
        }

        return blocks;
    }

    private enum Axis {
        X, Y, Z
    }
}
