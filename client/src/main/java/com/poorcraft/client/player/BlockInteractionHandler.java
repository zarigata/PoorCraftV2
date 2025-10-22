package com.poorcraft.client.player;

import com.poorcraft.client.input.InputManager;
import com.poorcraft.client.network.ClientNetworkManager;
import com.poorcraft.client.render.camera.Camera;
import com.poorcraft.client.world.World;
import com.poorcraft.common.Constants;
import com.poorcraft.common.config.Configuration;
import com.poorcraft.common.entity.Entity;
import com.poorcraft.common.entity.component.InteractionComponent;
import com.poorcraft.common.entity.component.InventoryComponent;
import com.poorcraft.common.entity.component.PositionComponent;
import com.poorcraft.common.inventory.ItemStack;
import com.poorcraft.common.inventory.ItemType;
import com.poorcraft.common.physics.AABB;
import com.poorcraft.common.physics.Raycast;
import com.poorcraft.common.physics.RaycastResult;
import com.poorcraft.common.world.block.BlockType;
import com.poorcraft.common.network.ConnectionState;
import com.poorcraft.common.network.packet.PlayerBlockPlacementPacket;
import com.poorcraft.common.network.packet.PlayerDiggingPacket;
import org.joml.Vector3f;

import static org.lwjgl.glfw.GLFW.GLFW_MOUSE_BUTTON_LEFT;
import static org.lwjgl.glfw.GLFW.GLFW_MOUSE_BUTTON_RIGHT;

/**
 * Handles block interaction for the local player.
 */
public class BlockInteractionHandler {

    private static final float DEFAULT_BREAK_TIME = 0.5f;
    private static final float PLACE_DELAY = 0.15f;
    private static final int ACTION_START = 0;
    private static final int ACTION_FINISH = 1;
    private static final int ACTION_CANCEL = 2;

    private final Entity player;
    private final World world;
    private final Camera camera;
    private final InputManager input;
    private final Configuration config;
    private final ClientNetworkManager networkManager;

    private RaycastResult currentTarget = RaycastResult.MISS;

    public BlockInteractionHandler(Entity player, World world, Camera camera, InputManager input, Configuration config, ClientNetworkManager networkManager) {
        this.player = player;
        this.world = world;
        this.camera = camera;
        this.input = input;
        this.config = config;
        this.networkManager = networkManager;
    }

    public void update(float dt) {
        InteractionComponent interaction = player.getComponent(InteractionComponent.class);
        InventoryComponent inventory = player.getComponent(InventoryComponent.class);
        PositionComponent position = player.getPosition();

        if (interaction == null || inventory == null || position == null) {
            return;
        }

        performRaycast(position);
        handleBreaking(interaction, dt);
        handlePlacement(interaction, inventory, position, dt);
    }

    private void performRaycast(PositionComponent position) {
        Vector3f origin = position.getPosition();
        origin.y += Constants.Player.EYE_HEIGHT;
        Vector3f direction = camera.getForward();
        currentTarget = Raycast.raycast(origin, direction, Constants.Player.REACH_DISTANCE, world);
    }

    private void handleBreaking(InteractionComponent interaction, float dt) {
        if (!currentTarget.hasHit()) {
            if (interaction.isBreaking()) {
                sendDiggingPacket(ACTION_CANCEL, interaction.getBreakBlockX(), interaction.getBreakBlockY(), interaction.getBreakBlockZ(), currentTarget);
                interaction.cancelBreaking();
            }
            return;
        }

        int targetX = currentTarget.getBlockX();
        int targetY = currentTarget.getBlockY();
        int targetZ = currentTarget.getBlockZ();

        if (!input.mouseDown(GLFW_MOUSE_BUTTON_LEFT)) {
            if (interaction.isBreaking()) {
                sendDiggingPacket(ACTION_CANCEL, interaction.getBreakBlockX(), interaction.getBreakBlockY(), interaction.getBreakBlockZ(), currentTarget);
                interaction.cancelBreaking();
            }
            return;
        }

        if (!interaction.isBreaking() || interaction.getBreakBlockX() != targetX || interaction.getBreakBlockY() != targetY || interaction.getBreakBlockZ() != targetZ) {
            float breakTime = getBlockBreakTime(currentTarget.getBlockType());
            interaction.startBreaking(targetX, targetY, targetZ, breakTime);
            sendDiggingPacket(ACTION_START, targetX, targetY, targetZ, currentTarget);
        }

        interaction.updateBreaking(dt);

        if (interaction.getBreakProgress() >= 1.0f) {
            if (networkManager != null && networkManager.isConnected() && networkManager.getConnectionState() == ConnectionState.PLAY) {
                sendDiggingPacket(ACTION_FINISH, targetX, targetY, targetZ, currentTarget);
            } else {
                world.setBlock(targetX, targetY, targetZ, BlockType.AIR.getId());
            }
            interaction.cancelBreaking();
        }
    }

    private float getBlockBreakTime(BlockType blockType) {
        if (blockType == null) {
            return DEFAULT_BREAK_TIME;
        }
        String key = "blocks." + blockType.getName() + ".breakTime";
        return config.getFloat(key, DEFAULT_BREAK_TIME);
    }

    private void handlePlacement(InteractionComponent interaction, InventoryComponent inventory, PositionComponent position, float dt) {
        if (!input.mousePressed(GLFW_MOUSE_BUTTON_RIGHT)) {
            return;
        }

        ItemStack selected = inventory.getSelectedItem();
        if (selected == null || selected.isEmpty()) {
            return;
        }

        ItemType itemType = selected.getItemType();
        if (!itemType.isPlaceable()) {
            return;
        }

        long now = System.currentTimeMillis();
        if (!interaction.canPlace(now, PLACE_DELAY)) {
            return;
        }

        RaycastResult target = currentTarget;
        if (!target.hasHit()) {
            return;
        }

        int[] adjacent = target.getAdjacentBlockPos();
        int placeX = adjacent[0];
        int placeY = adjacent[1];
        int placeZ = adjacent[2];

        if (!canPlaceBlock(position, placeX, placeY, placeZ)) {
            return;
        }

        BlockType blockType = itemType.getBlockType();
        if (blockType == null) {
            return;
        }

        if (networkManager != null && networkManager.isConnected() && networkManager.getConnectionState() == ConnectionState.PLAY) {
            sendPlacementPacket(target, blockType);
        } else {
            world.setBlock(placeX, placeY, placeZ, blockType.getId());
            selected.removeFromStack(1);
            if (selected.isEmpty()) {
                int selectedSlot = inventory.getInventory().getSelectedSlot();
                inventory.getInventory().setHotbarSlot(selectedSlot, ItemStack.EMPTY.copy());
            }
        }
        interaction.recordPlace(now);
    }

    private void sendDiggingPacket(int action, int x, int y, int z, RaycastResult hit) {
        if (networkManager == null) {
            return;
        }
        if (!networkManager.isConnected() || networkManager.getConnectionState() != ConnectionState.PLAY) {
            return;
        }
        PlayerDiggingPacket packet = new PlayerDiggingPacket(action, x, y, z, faceFromHit(hit));
        networkManager.sendPacket(packet);
    }

    private int faceFromHit(RaycastResult hit) {
        int faceX = hit.getFaceX();
        int faceY = hit.getFaceY();
        int faceZ = hit.getFaceZ();
        if (faceY == -1) return 0;
        if (faceY == 1) return 1;
        if (faceZ == -1) return 2;
        if (faceZ == 1) return 3;
        if (faceX == -1) return 4;
        if (faceX == 1) return 5;
        return 1;
    }

    private void sendPlacementPacket(RaycastResult target, BlockType blockType) {
        if (networkManager == null) {
            return;
        }

        PlayerBlockPlacementPacket packet = new PlayerBlockPlacementPacket(
            target.getBlockX(),
            target.getBlockY(),
            target.getBlockZ(),
            faceFromHit(target),
            0,
            target.getHitPos().x - target.getBlockX(),
            target.getHitPos().y - target.getBlockY(),
            target.getHitPos().z - target.getBlockZ()
        );
        networkManager.sendPacket(packet);
    }

    private boolean canPlaceBlock(PositionComponent position, int x, int y, int z) {
        Vector3f playerPos = position.getPosition();
        AABB playerBox = AABB.fromEntity(playerPos, Constants.Player.WIDTH, Constants.Player.HEIGHT);
        AABB blockBox = new AABB(x, y, z, x + 1.0f, y + 1.0f, z + 1.0f);
        return !playerBox.intersects(blockBox);
    }

    public RaycastResult getCurrentTarget() {
        return currentTarget;
    }
}
