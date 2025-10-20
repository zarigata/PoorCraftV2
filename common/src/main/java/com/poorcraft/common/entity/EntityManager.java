package com.poorcraft.common.entity;

import com.poorcraft.common.entity.behavior.BehaviorStateMachine;
import com.poorcraft.common.entity.behavior.IdleBehavior;
import com.poorcraft.common.entity.component.AnimationComponent;
import com.poorcraft.common.entity.component.BehaviorComponent;
import com.poorcraft.common.entity.component.NameComponent;
import com.poorcraft.common.entity.component.PositionComponent;
import com.poorcraft.common.entity.component.SkinComponent;
import com.poorcraft.common.entity.component.VelocityComponent;
import com.poorcraft.common.util.ChunkPos;
import org.joml.Vector3f;

import java.util.Collection;
import java.util.Collections;
import java.util.EnumMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;

public class EntityManager {
    private final Map<UUID, Entity> entities;
    private final Map<EntityType, Set<Entity>> entitiesByType;
    private final Map<ChunkPos, Set<Entity>> spatialIndex;
    private final Map<UUID, BehaviorStateMachine> behaviorStateMachines;
    private final AtomicInteger nextEntityId;

    public EntityManager() {
        this.entities = new ConcurrentHashMap<>();
        this.entitiesByType = new EnumMap<>(EntityType.class);
        this.spatialIndex = new ConcurrentHashMap<>();
        this.behaviorStateMachines = new ConcurrentHashMap<>();
        this.nextEntityId = new AtomicInteger();
    }

    public Entity addEntity(Entity entity) {
        if (entity == null) {
            return null;
        }
        entities.put(entity.getId(), entity);
        entitiesByType.computeIfAbsent(entity.getType(), ignored -> ConcurrentHashMap.newKeySet()).add(entity);
        PositionComponent position = entity.getPosition();
        if (position != null) {
            ChunkPos chunkPos = ChunkPos.fromWorldPos(position.getPosition().x, position.getPosition().z);
            spatialIndex.computeIfAbsent(chunkPos, ignored -> ConcurrentHashMap.newKeySet()).add(entity);
        }
        return entity;
    }

    public Entity removeEntity(UUID id) {
        Entity removed = entities.remove(id);
        if (removed == null) {
            return null;
        }
        Set<Entity> typed = entitiesByType.get(removed.getType());
        if (typed != null) {
            typed.remove(removed);
        }
        for (Set<Entity> bucket : spatialIndex.values()) {
            bucket.remove(removed);
        }
        behaviorStateMachines.remove(id);
        return removed;
    }

    public Entity getEntity(UUID id) {
        return entities.get(id);
    }

    public Collection<Entity> getAllEntities() {
        return Collections.unmodifiableCollection(entities.values());
    }

    public Set<Entity> getEntitiesByType(EntityType type) {
        Set<Entity> set = entitiesByType.get(type);
        if (set == null) {
            return Collections.emptySet();
        }
        return Collections.unmodifiableSet(set);
    }

    public Set<Entity> getEntitiesInChunk(ChunkPos pos) {
        Set<Entity> set = spatialIndex.get(pos);
        if (set == null) {
            return Collections.emptySet();
        }
        return Collections.unmodifiableSet(set);
    }

    public Set<Entity> getEntitiesInRadius(Vector3f center, float radius) {
        float radiusSq = radius * radius;
        Set<Entity> result = new HashSet<>();
        for (Entity entity : entities.values()) {
            PositionComponent position = entity.getPosition();
            if (position == null) {
                continue;
            }
            Vector3f pos = position.getPosition();
            if (pos.distanceSquared(center) <= radiusSq) {
                result.add(entity);
            }
        }
        return result;
    }

    public void clear() {
        entities.clear();
        entitiesByType.clear();
        spatialIndex.clear();
        behaviorStateMachines.clear();
        nextEntityId.set(0);
    }

    public void update(double dt) {
        for (Entity entity : entities.values()) {
            PositionComponent position = entity.getPosition();
            VelocityComponent velocity = entity.getVelocity();
            if (position != null) {
                position.updatePrevious();
            }
            if (velocity != null) {
                Vector3f pos = position != null ? position.getPosition() : null;
                if (pos != null) {
                    Vector3f vel = velocity.getVelocity();
                    pos.add(new Vector3f(vel).mul((float) dt));
                    position.setPosition(pos);
                }
                velocity.applyDrag();
                velocity.clampSpeed();
            }
            AnimationComponent animation = entity.getComponent(AnimationComponent.class);
            if (animation != null) {
                animation.updateTime((float) dt);
            }
            BehaviorComponent behavior = entity.getComponent(BehaviorComponent.class);
            if (behavior != null && behavior.isAIEnabled()) {
                BehaviorStateMachine machine = behaviorStateMachines.computeIfAbsent(entity.getId(), ignored -> {
                    BehaviorStateMachine created = new BehaviorStateMachine(entity);
                    created.registerState("idle", new IdleBehavior());
                    created.setState("idle");
                    return created;
                });
                machine.update(dt);
            }
        }
    }

    public Entity createPlayer(String name, Vector3f position) {
        Entity entity = new Entity(EntityType.PLAYER);
        entity.addComponent(new PositionComponent(position));
        entity.addComponent(new VelocityComponent());
        entity.addComponent(new SkinComponent(SkinComponent.getDefaultSkin()));
        entity.addComponent(new NameComponent(name));
        entity.addComponent(new AnimationComponent());
        addEntity(entity);
        return entity;
    }

    public Entity createNPC(String name, Vector3f position, String behaviorType) {
        Entity entity = new Entity(EntityType.NPC);
        entity.addComponent(new PositionComponent(position));
        entity.addComponent(new VelocityComponent());
        entity.addComponent(new SkinComponent("skins/npc_villager.png"));
        entity.addComponent(new NameComponent(name));
        entity.addComponent(new AnimationComponent());
        BehaviorComponent behavior = new BehaviorComponent(behaviorType);
        entity.addComponent(behavior);
        addEntity(entity);
        BehaviorStateMachine machine = new BehaviorStateMachine(entity);
        machine.registerState("idle", new IdleBehavior());
        machine.setState("idle");
        behaviorStateMachines.put(entity.getId(), machine);
        return entity;
    }

    public BehaviorStateMachine getBehaviorStateMachine(UUID entityId) {
        return behaviorStateMachines.get(entityId);
    }

    public int getNextEntityId() {
        return nextEntityId.incrementAndGet();
    }
}
