package com.poorcraft.client.player;

import com.poorcraft.client.input.InputManager;
import com.poorcraft.client.network.ClientNetworkManager;
import com.poorcraft.client.render.camera.Camera;
import com.poorcraft.common.Constants;
import com.poorcraft.common.config.Configuration;
import com.poorcraft.common.entity.Entity;
import com.poorcraft.common.entity.component.PhysicsComponent;
import com.poorcraft.common.entity.component.PositionComponent;
import com.poorcraft.common.entity.component.VelocityComponent;
import com.poorcraft.common.network.ConnectionState;
import com.poorcraft.common.network.packet.PlayerPositionLookPacket;
import org.joml.Vector3f;

import static org.lwjgl.glfw.GLFW.*;

/**
 * Handles local player input and movement.
 */
public class PlayerController {

    private static final float DEFAULT_MOUSE_SENSITIVITY = 0.15f;
    private static final float JUMP_COOLDOWN = 0.2f;

    private final Entity player;
    private final Camera camera;
    private final InputManager input;
    private final Configuration config;
    private final ClientNetworkManager networkManager;

    private final Vector3f moveDir = new Vector3f();
    private float mouseSensitivity;
    private long lastMovementSendTime;

    public PlayerController(Entity player, Camera camera, InputManager input, Configuration config, ClientNetworkManager networkManager) {
        this.player = player;
        this.camera = camera;
        this.input = input;
        this.config = config;
        this.networkManager = networkManager;
        this.mouseSensitivity = config.getFloat("input.mouseSensitivity", DEFAULT_MOUSE_SENSITIVITY);
    }

    public void update(float dt) {
        PositionComponent position = player.getPosition();
        VelocityComponent velocity = player.getVelocity();
        PhysicsComponent physics = player.getComponent(PhysicsComponent.class);
        if (position == null || velocity == null || physics == null) {
            return;
        }

        handleLook(position);
        handleMovement(velocity, physics, position, dt);
        updateCamera(position);
        sendMovementUpdate(position);
    }

    private void handleLook(PositionComponent position) {
        float dx = (float) input.getDeltaMouseX();
        float dy = (float) input.getDeltaMouseY();
        if (dx == 0.0f && dy == 0.0f) {
            return;
        }

        float yaw = position.getYaw() - dx * mouseSensitivity;
        float pitch = position.getPitch() - dy * mouseSensitivity;
        pitch = Math.max(-89.0f, Math.min(89.0f, pitch));
        position.setRotation(yaw, pitch);
        camera.setRotation(yaw, pitch);
    }

    private void handleMovement(VelocityComponent velocity, PhysicsComponent physics, PositionComponent position, float dt) {
        moveDir.zero();
        Vector3f forward = camera.getForward();
        Vector3f right = camera.getRight();
        forward.y = 0.0f;
        right.y = 0.0f;
        forward.normalize();
        right.normalize();

        if (input.keyDown(GLFW_KEY_W)) {
            moveDir.add(forward);
        }
        if (input.keyDown(GLFW_KEY_S)) {
            moveDir.sub(forward);
        }
        if (input.keyDown(GLFW_KEY_D)) {
            moveDir.add(right);
        }
        if (input.keyDown(GLFW_KEY_A)) {
            moveDir.sub(right);
        }

        boolean sprinting = input.keyDown(GLFW_KEY_LEFT_SHIFT) || input.keyDown(GLFW_KEY_RIGHT_SHIFT);
        boolean crouching = input.keyDown(GLFW_KEY_LEFT_CONTROL) || input.keyDown(GLFW_KEY_RIGHT_CONTROL);

        physics.setCrouching(crouching);
        physics.setSprinting(sprinting);

        float speed = Constants.Player.WALK_SPEED;
        if (sprinting && !crouching) {
            speed = Constants.Player.SPRINT_SPEED;
        } else if (crouching) {
            speed = Constants.Player.SNEAK_SPEED;
        }

        if (moveDir.lengthSquared() > 0.0f) {
            moveDir.normalize().mul(speed);
        }

        Vector3f vel = velocity.getVelocity();
        vel.x = moveDir.x;
        vel.z = moveDir.z;

        physics.updateJumpCooldown(dt);
        boolean wantsJump = input.keyPressed(GLFW_KEY_SPACE);
        if (wantsJump && position.isOnGround() && physics.canJumpNow()) {
            vel.y = Constants.Physics.JUMP_VELOCITY;
            physics.resetJumpCooldown(JUMP_COOLDOWN);
        }

        velocity.setVelocity(vel);
    }

    private void updateCamera(PositionComponent position) {
        Vector3f camPos = position.getPosition();
        camPos.y += Constants.Player.EYE_HEIGHT;
        camera.setPosition(camPos);
    }

    private void sendMovementUpdate(PositionComponent position) {
        if (networkManager == null || !networkManager.isConnected()) {
            return;
        }

        if (networkManager.getConnectionState() != ConnectionState.PLAY) {
            return;
        }

        long now = System.currentTimeMillis();
        if (now - lastMovementSendTime < 50L) {
            return;
        }

        lastMovementSendTime = now;

        Vector3f pos = position.getPosition();
        PlayerPositionLookPacket packet = new PlayerPositionLookPacket(
            pos.x,
            pos.y,
            pos.z,
            position.getYaw(),
            position.getPitch(),
            position.isOnGround()
        );
        networkManager.sendPacket(packet);
    }
}
