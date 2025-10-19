#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "poorcraft/entity/EntityManager.h"

namespace PoorCraft {

class Camera;
class Shader;

class EntityRenderer {
public:
    explicit EntityRenderer(EntityManager& manager);

    void render(const Camera& camera, Shader& shader, float interpolationAlpha);
    void setDepthSorting(bool enabled);

private:
    using EntityList = std::vector<Entity*>;

    void sortEntitiesByDepth(EntityList& entities, const glm::vec3& cameraPosition) const;
    void renderEntity(Entity& entity, const Camera& camera, Shader& shader, float interpolationAlpha);

    EntityManager& m_Manager;
    bool m_DepthSorting = true;
};

} // namespace PoorCraft
