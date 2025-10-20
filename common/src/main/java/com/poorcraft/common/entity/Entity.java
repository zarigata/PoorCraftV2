package com.poorcraft.common.entity;

import com.poorcraft.common.entity.component.PositionComponent;
import com.poorcraft.common.entity.component.VelocityComponent;

import java.lang.ref.WeakReference;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
public class Entity {
    private final UUID id;
    private final EntityType type;
    private final Map<Class<? extends Component>, Component> components;
    private WeakReference<Object> worldRef;
    private boolean active;

    public Entity(EntityType type) {
        this.id = UUID.randomUUID();
        this.type = Objects.requireNonNull(type, "type");
        this.components = new ConcurrentHashMap<>();
        this.active = true;
    }

    public UUID getId() {
        return id;
    }

    public EntityType getType() {
        return type;
    }

    public boolean isActive() {
        return active;
    }

    public void setActive(boolean active) {
        this.active = active;
    }

    public void setWorld(Object world) {
        this.worldRef = world == null ? null : new WeakReference<>(world);
    }

    public Object getWorld() {
        return worldRef == null ? null : worldRef.get();
    }

    public <T extends Component> void addComponent(T component) {
        Objects.requireNonNull(component, "component");
        components.put(component.getClass(), component);
    }

    public <T extends Component> T removeComponent(Class<T> type) {
        Objects.requireNonNull(type, "type");
        Component removed = components.remove(type);
        return type.cast(removed);
    }

    public <T extends Component> T getComponent(Class<T> type) {
        Objects.requireNonNull(type, "type");
        Component component = components.get(type);
        return component == null ? null : type.cast(component);
    }

    public boolean hasComponent(Class<? extends Component> type) {
        Objects.requireNonNull(type, "type");
        return components.containsKey(type);
    }

    public PositionComponent getPosition() {
        return getComponent(PositionComponent.class);
    }

    public VelocityComponent getVelocity() {
        return getComponent(VelocityComponent.class);
    }

    public Map<Class<? extends Component>, Component> getComponents() {
        return Map.copyOf(components);
    }

    public Optional<Component> findComponent(Class<? extends Component> type) {
        return Optional.ofNullable(components.get(type));
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (!(obj instanceof Entity other)) {
            return false;
        }
        return id.equals(other.id);
    }

    @Override
    public int hashCode() {
        return id.hashCode();
    }

    @Override
    public String toString() {
        return "Entity{" +
                "id=" + id +
                ", type=" + type +
                ", active=" + active +
                '}';
    }
}
