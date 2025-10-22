package com.poorcraft.client.network.handler;

import com.poorcraft.client.network.ClientConnection;
import com.poorcraft.client.world.World;
import com.poorcraft.common.entity.Entity;
import com.poorcraft.common.entity.EntityManager;
import com.poorcraft.common.entity.EntityType;
import com.poorcraft.common.entity.component.PhysicsComponent;
import com.poorcraft.common.entity.component.PositionComponent;
import com.poorcraft.common.entity.component.VelocityComponent;
import com.poorcraft.common.network.NetworkConnection;
import com.poorcraft.common.network.PacketHandler;
import com.poorcraft.common.network.packet.EntityPositionPacket;
import com.poorcraft.common.network.packet.EntityRemovePacket;
import com.poorcraft.common.network.packet.EntitySpawnPacket;
import com.poorcraft.common.network.packet.EntityVelocityPacket;
import org.joml.Vector3f;
import org.slf4j.Logger;

import java.util.Map;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Handles entity update packets from server.
 * <p>
 * Processes entity spawn, position, velocity, and removal updates.
 */
public class EntityUpdateHandler implements PacketHandler {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(EntityUpdateHandler.class);

    private final World world;
    private final EntityManager entityManager;
    private final Map<Integer, UUID> serverToClientIds = new ConcurrentHashMap<>();

    public EntityUpdateHandler(World world, EntityManager entityManager) {
        this.world = world;
        this.entityManager = entityManager;
    }

    @Override
    public void handle(Object packet, NetworkConnection connection) {
        if (!(connection instanceof ClientConnection)) {
            LOGGER.error("EntityUpdateHandler received non-client connection");
            return;
        }

        if (packet instanceof EntitySpawnPacket) {
            handleEntitySpawn((EntitySpawnPacket) packet, connection);
        } else if (packet instanceof EntityPositionPacket) {
            handleEntityPosition((EntityPositionPacket) packet, connection);
        } else if (packet instanceof EntityVelocityPacket) {
            handleEntityVelocity((EntityVelocityPacket) packet, connection);
        } else if (packet instanceof EntityRemovePacket) {
            handleEntityRemove((EntityRemovePacket) packet, connection);
        }
    }

    /**
     * Handles entity spawn packets.
     *
     * @param packet the spawn packet
     * @param connection the connection
     */
    private void handleEntitySpawn(EntitySpawnPacket packet, NetworkConnection connection) {
        LOGGER.debug("Entity spawn: ID={}, UUID={}, Type={}, Pos=({},{},{})",
            packet.getEntityId(), packet.getEntityUuid(), packet.getType(),
            packet.getX(), packet.getY(), packet.getZ());

        UUID previousId = serverToClientIds.remove(packet.getEntityId());
        if (previousId != null) {
            entityManager.removeEntity(previousId);
        }

        EntityType type = EntityType.getById(packet.getType());
        if (type == null) {
            LOGGER.warn("Unknown entity type {} for spawn packet {}", packet.getType(), packet.getEntityId());
            return;
        }

        Entity entity = createEntity(type, packet);
        if (entity == null) {
            LOGGER.warn("Failed to create entity for spawn packet {}", packet.getEntityId());
            return;
        }

        entity.setWorld(world);

        PositionComponent position = entity.getPosition();
        if (position != null) {
            position.setPosition((float) packet.getX(), (float) packet.getY(), (float) packet.getZ());
            position.setRotation(packet.getYaw(), packet.getPitch());
            position.updatePrevious();
        }

        VelocityComponent velocity = entity.getVelocity();
        if (velocity != null) {
            velocity.setVelocity((float) packet.getVelocityX(),
                (float) packet.getVelocityY(), (float) packet.getVelocityZ());
        }

        serverToClientIds.put(packet.getEntityId(), entity.getId());

        LOGGER.debug("Processed entity spawn for ID {}", packet.getEntityId());
    }

    /**
     * Handles entity position packets.
     *
     * @param packet the position packet
     * @param connection the connection
     */
    private void handleEntityPosition(EntityPositionPacket packet, NetworkConnection connection) {
        LOGGER.debug("Entity position update: ID={}, Delta=({},{},{})",
            packet.getEntityId(), packet.getDeltaX(), packet.getDeltaY(), packet.getDeltaZ());

        Entity entity = resolveEntity(packet.getEntityId());
        if (entity == null) {
            LOGGER.debug("Entity position update skipped, unknown entity {}", packet.getEntityId());
            return;
        }

        PositionComponent position = entity.getPosition();
        if (position == null) {
            LOGGER.debug("Entity {} missing PositionComponent", entity.getId());
            return;
        }

        Vector3f prev = position.getPosition();
        position.updatePrevious();
        position.setPosition(prev.x + (float) packet.getDeltaX(),
            prev.y + (float) packet.getDeltaY(), prev.z + (float) packet.getDeltaZ());
        position.setOnGround(packet.isOnGround());

        LOGGER.debug("Processed entity position for ID {}", packet.getEntityId());
    }

    /**
     * Handles entity velocity packets.
     *
     * @param packet the velocity packet
     * @param connection the connection
     */
    private void handleEntityVelocity(EntityVelocityPacket packet, NetworkConnection connection) {
        LOGGER.debug("Entity velocity update: ID={}, Vel=({},{},{})",
            packet.getEntityId(), packet.getVelocityX(), packet.getVelocityY(), packet.getVelocityZ());

        Entity entity = resolveEntity(packet.getEntityId());
        if (entity == null) {
            LOGGER.debug("Entity velocity update skipped, unknown entity {}", packet.getEntityId());
            return;
        }

        VelocityComponent velocity = entity.getVelocity();
        if (velocity == null) {
            velocity = new VelocityComponent();
            entity.addComponent(velocity);
        }

        velocity.setVelocity((float) packet.getVelocityX(),
            (float) packet.getVelocityY(), (float) packet.getVelocityZ());

        LOGGER.debug("Processed entity velocity for ID {}", packet.getEntityId());
    }

    /**
     * Handles entity removal packets.
     *
     * @param packet the removal packet
     * @param connection the connection
     */
    private void handleEntityRemove(EntityRemovePacket packet, NetworkConnection connection) {
        LOGGER.debug("Entity removal: {}", java.util.Arrays.toString(packet.getEntityIds()));

        for (int serverId : packet.getEntityIds()) {
            UUID clientId = serverToClientIds.remove(serverId);
            if (clientId == null) {
                continue;
            }

            Entity removed = entityManager.removeEntity(clientId);
            if (removed == null) {
                continue;
            }

            removed.setWorld(null);
        }

        LOGGER.debug("Processed entity removal");
    }

    private Entity resolveEntity(int serverEntityId) {
        UUID clientId = serverToClientIds.get(serverEntityId);
        if (clientId == null) {
            return null;
        }
        return entityManager.getEntity(clientId);
    }

    private Entity createEntity(EntityType type, EntitySpawnPacket packet) {
        switch (type) {
            case PLAYER:
                return entityManager.createPlayer(packet.getEntityUuid().toString(),
                    new Vector3f((float) packet.getX(), (float) packet.getY(), (float) packet.getZ()));
            case NPC:
                return entityManager.createNPC(packet.getEntityUuid().toString(),
                    new Vector3f((float) packet.getX(), (float) packet.getY(), (float) packet.getZ()), "idle");
            default:
                Entity entity = new Entity(type);
                entity.addComponent(new PositionComponent(new Vector3f((float) packet.getX(),
                    (float) packet.getY(), (float) packet.getZ())));
                entity.addComponent(new VelocityComponent());
                entity.addComponent(new PhysicsComponent());
                entityManager.addEntity(entity);
                return entity;
        }
    }
}
