package com.poorcraft.client.entity;

import com.poorcraft.client.render.camera.Camera;
import com.poorcraft.client.render.camera.Frustum;
import com.poorcraft.client.render.shader.ShaderProgram;
import com.poorcraft.client.render.texture.Texture2D;
import com.poorcraft.client.resource.ShaderManager;
import com.poorcraft.client.resource.TextureManager;
import com.poorcraft.common.config.Configuration;
import com.poorcraft.common.entity.Entity;
import com.poorcraft.common.entity.EntityManager;
import com.poorcraft.common.entity.EntityType;
import com.poorcraft.common.entity.component.PositionComponent;
import com.poorcraft.common.entity.component.SkinComponent;
import org.joml.Matrix4f;
import org.joml.Quaternionf;
import org.joml.Vector3f;
import org.slf4j.Logger;

import java.util.Collection;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;

import static org.lwjgl.opengl.GL13.GL_TEXTURE0;
import static org.lwjgl.opengl.GL13.glActiveTexture;

public class EntityRenderer {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(EntityRenderer.class);
    private static final float MODEL_MIN_Y = -0.75f;

    private final EntityManager entityManager;
    private final Camera camera;
    private final Frustum frustum;
    private final ShaderProgram entityShader;
    private final EntityModel playerModel;
    private final SkinLoader skinLoader;
    private final Texture2D missingTexture;
    private final Map<UUID, Texture2D> entitySkins;
    private final Matrix4f modelMatrix;
    private final Matrix4f viewProjectionMatrix;
    private final Vector3f tmpMin;
    private final Vector3f tmpMax;
    private final Vector3f lightDirection;
    private final Vector3f lightColor;
    private final Vector3f ambientColor;
    private final Vector3f fogColor;
    private final float fogStart;
    private final float fogEnd;
    private final Set<UUID> pendingSkinLoads;

    public EntityRenderer(EntityManager entityManager,
                          Camera camera,
                          ShaderManager shaderManager,
                          TextureManager textureManager,
                          Configuration config) {
        this.entityManager = entityManager;
        this.camera = camera;
        this.frustum = new Frustum();
        this.playerModel = EntityModel.buildPlayerModel();
        this.skinLoader = new SkinLoader(textureManager, config);
        this.missingTexture = textureManager.getMissingTexture();
        this.entitySkins = new ConcurrentHashMap<>();
        this.pendingSkinLoads = ConcurrentHashMap.newKeySet();
        this.modelMatrix = new Matrix4f();
        this.viewProjectionMatrix = new Matrix4f();
        this.tmpMin = new Vector3f();
        this.tmpMax = new Vector3f();
        this.lightDirection = new Vector3f(-0.3f, -1.0f, -0.2f).normalize();
        this.lightColor = new Vector3f(1.0f, 0.95f, 0.8f);
        this.ambientColor = new Vector3f(0.4f, 0.4f, 0.5f);
        float fogStartConfig = config.getFloat("graphics.fogStart", 100.0f);
        float fogEndConfig = config.getFloat("graphics.fogEnd", 200.0f);
        this.fogStart = fogStartConfig;
        this.fogEnd = Math.max(fogEndConfig, fogStartConfig + 1.0f);
        float fogR = config.getFloat("graphics.fogColorR", 0.5f);
        float fogG = config.getFloat("graphics.fogColorG", 0.7f);
        float fogB = config.getFloat("graphics.fogColorB", 1.0f);
        this.fogColor = new Vector3f(fogR, fogG, fogB);

        try {
            this.entityShader = shaderManager.loadShader("entity");
        } catch (Exception e) {
            LOGGER.error("Failed to load entity shader", e);
            throw new RuntimeException("Failed to initialize EntityRenderer", e);
        }
        this.playerModel.upload();
    }

    public void render(double alpha) {
        skinLoader.pumpRenderQueue();

        camera.getProjectionMatrix().mul(camera.getViewMatrix(), viewProjectionMatrix);
        frustum.extractPlanes(viewProjectionMatrix);

        entityShader.use();
        entityShader.setMatrix4f("uView", camera.getViewMatrix());
        entityShader.setMatrix4f("uProjection", camera.getProjectionMatrix());
        entityShader.setVector3f("uLightDir", lightDirection);
        entityShader.setVector3f("uLightColor", lightColor);
        entityShader.setVector3f("uAmbientColor", ambientColor);
        entityShader.setVector3f("uFogColor", fogColor);
        entityShader.setFloat("uFogStart", fogStart);
        entityShader.setFloat("uFogEnd", fogEnd);
        entityShader.setVector3f("uCameraPos", camera.getPosition());
        entityShader.setInt("uSkinTexture", 0);

        glActiveTexture(GL_TEXTURE0);

        Collection<Entity> entities = entityManager.getAllEntities();
        for (Entity entity : entities) {
            if (entity == null || !entity.isActive()) {
                continue;
            }

            PositionComponent position = entity.getPosition();
            if (position == null) {
                continue;
            }

            Vector3f interpolatedPos = position.getInterpolatedPosition(alpha);
            Quaternionf interpolatedRot = position.getInterpolatedRotation(alpha);

            EntityType type = entity.getType();
            float halfWidth = type.getWidth() * 0.5f;
            tmpMin.set(interpolatedPos.x - halfWidth, interpolatedPos.y, interpolatedPos.z - halfWidth);
            tmpMax.set(interpolatedPos.x + halfWidth, interpolatedPos.y + type.getHeight(), interpolatedPos.z + halfWidth);
            if (!frustum.testAABB(tmpMin, tmpMax)) {
                continue;
            }

            float scaleY = type.getHeight() / 2.0f;
            float scaleXZ = type.getWidth();
            float baseOffsetY = -MODEL_MIN_Y * scaleY;

            modelMatrix.identity()
                    .translate(interpolatedPos.x, interpolatedPos.y + baseOffsetY, interpolatedPos.z)
                    .rotate(interpolatedRot)
                    .scale(scaleXZ, scaleY, scaleXZ);

            entityShader.setMatrix4f("uModel", modelMatrix);

            Texture2D texture = resolveTexture(entity);
            texture.bind(0);

            playerModel.render();
        }

        pruneSkinCache(entities);
    }

    private Texture2D resolveTexture(Entity entity) {
        Texture2D cached = entitySkins.get(entity.getId());
        if (cached != null) {
            return cached;
        }

        SkinComponent skinComponent = entity.getComponent(SkinComponent.class);
        final SkinComponent trackedSkin = skinComponent;
        String skinPath = trackedSkin != null ? trackedSkin.getSkinPath() : null;
        UUID id = entity.getId();
        if (pendingSkinLoads.add(id)) {
            skinLoader.loadSkin(skinPath, texture -> {
                entitySkins.put(id, texture);
                pendingSkinLoads.remove(id);
                if (trackedSkin != null) {
                    trackedSkin.setLoaded(true);
                }
            });
        }
        return missingTexture;
    }

    private void pruneSkinCache(Collection<Entity> entities) {
        Set<UUID> existingIds = new HashSet<>();
        for (Entity entity : entities) {
            if (entity != null) {
                existingIds.add(entity.getId());
            }
        }
        entitySkins.keySet().removeIf(id -> !existingIds.contains(id));
        pendingSkinLoads.removeIf(id -> !existingIds.contains(id));
    }

    public void cleanup() {
        playerModel.cleanup();
        skinLoader.cleanup();
        entitySkins.clear();
        pendingSkinLoads.clear();
    }
}
