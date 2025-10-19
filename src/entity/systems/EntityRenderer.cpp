#include "poorcraft/entity/systems/EntityRenderer.h"

#include <algorithm>
#include <array>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "poorcraft/core/Logger.h"
#include "poorcraft/entity/components/Renderable.h"
#include "poorcraft/entity/components/Transform.h"
#include "poorcraft/entity/components/AnimationController.h"
#include "poorcraft/rendering/Camera.h"
#include "poorcraft/rendering/Shader.h"
#include "poorcraft/rendering/Texture.h"

namespace PoorCraft {

EntityRenderer::EntityRenderer(EntityManager& manager)
    : m_Manager(manager) {}

void EntityRenderer::render(const Camera& camera, Shader& shader, float interpolationAlpha) {
    auto entities = m_Manager.getEntitiesWith<Renderable>();
    if (entities.empty()) {
        return;
    }

    EntityList opaqueEntities;
    EntityList transparentEntities;
    opaqueEntities.reserve(entities.size());
    transparentEntities.reserve(entities.size());

    for (auto* entity : entities) {
        if (!entity) {
            continue;
        }

        auto* renderable = entity->getComponent<Renderable>();
        auto* transform = entity->getComponent<Transform>();
        if (!renderable || !transform) {
            continue;
        }

        if (!renderable->isVisible()) {
            continue;
        }

        auto mesh = renderable->getMesh();
        auto texture = renderable->getTexture();
        if (!mesh || !texture) {
            continue;
        }

        if (renderable->getRenderLayer() == 1) {
            transparentEntities.push_back(entity);
        } else {
            opaqueEntities.push_back(entity);
        }
    }

    if (opaqueEntities.empty() && transparentEntities.empty()) {
        return;
    }

    auto renderList = [&](const EntityList& list) {
        for (auto* entity : list) {
            renderEntity(*entity, camera, shader, interpolationAlpha);
        }
    };

    renderList(opaqueEntities);

    if (m_DepthSorting && !transparentEntities.empty()) {
        sortEntitiesByDepth(transparentEntities, camera.getPosition());
    }

    renderList(transparentEntities);
}

void EntityRenderer::setDepthSorting(bool enabled) {
    m_DepthSorting = enabled;
    PC_INFO("EntityRenderer depth sorting set to " + std::string(enabled ? "enabled" : "disabled"));
}

void EntityRenderer::sortEntitiesByDepth(EntityList& entities, const glm::vec3& cameraPosition) const {
    std::sort(entities.begin(), entities.end(), [&](Entity* a, Entity* b) {
        auto* transformA = a->getComponent<Transform>();
        auto* transformB = b->getComponent<Transform>();
        if (!transformA || !transformB) {
            return false;
        }

        const glm::vec3 posA = transformA->getPosition();
        const glm::vec3 posB = transformB->getPosition();
        const float distA = glm::length2(cameraPosition - posA);
        const float distB = glm::length2(cameraPosition - posB);
        return distA > distB;
    });
}

void EntityRenderer::renderEntity(Entity& entity, const Camera& camera, Shader& shader, float interpolationAlpha) {
    auto* transform = entity.getComponent<Transform>();
    auto* renderable = entity.getComponent<Renderable>();
    if (!transform || !renderable) {
        return;
    }

    auto mesh = renderable->getMesh();
    auto texture = renderable->getTexture();
    if (!mesh || !texture) {
        return;
    }

    glm::vec3 interpolatedPosition = transform->getInterpolatedPosition(interpolationAlpha);
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), interpolatedPosition);
    glm::mat4 rotation = glm::toMat4(transform->getRotation());
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), transform->getScale());
    glm::mat4 baseModel = translation * rotation * scale;

    shader.setMat4("view", camera.getViewMatrix());
    shader.setMat4("projection", camera.getProjectionMatrix());

    texture->bind(0);
    shader.setInt("skinTexture", 0);

    const auto& sections = renderable->getSections();
    auto* animation = entity.getComponent<AnimationController>();
    const bool hasAnimation = animation != nullptr;

    if (!sections.empty() && hasAnimation) {
        static const std::array<const char*, 6> BONE_NAMES = {
            "head",
            "body",
            "leftArm",
            "rightArm",
            "leftLeg",
            "rightLeg"
        };

        const std::size_t boneCount = std::min(sections.size(), BONE_NAMES.size());
        for (std::size_t i = 0; i < boneCount; ++i) {
            const auto& section = sections[i];
            if (section.indexCount == 0) {
                continue;
            }

            const auto& boneTransform = animation->getBoneTransform(BONE_NAMES[i]);
            glm::mat4 limbTransform = glm::translate(glm::mat4(1.0f), boneTransform.position) * glm::toMat4(boneTransform.rotation);
            glm::mat4 model = baseModel * limbTransform;
            shader.setMat4("model", model);
            mesh->draw(GL_TRIANGLES, section.indexCount, section.indexOffset);
        }
    } else {
        shader.setMat4("model", baseModel);
        const std::size_t count = mesh->getIndexCount();
        mesh->draw(GL_TRIANGLES, count);
    }
}

} // namespace PoorCraft
