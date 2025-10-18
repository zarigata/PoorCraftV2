#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "poorcraft/physics/AABB.h"
#include "poorcraft/rendering/Camera.h"

namespace PoorCraft {

class Shader;

class DebugRenderer {
public:
    static DebugRenderer& getInstance();

    void initialize();
    void shutdown();

    void setEnabled(bool enabled);
    [[nodiscard]] bool isEnabled() const;

    void clear();

    void drawAABB(const PhysicsAABB& bounds, const glm::vec3& color);
    void drawRay(const glm::vec3& origin,
                 const glm::vec3& direction,
                 float length,
                 const glm::vec3& color);

    void render(const Camera& camera);

private:
    struct DebugAABB {
        PhysicsAABB bounds;
        glm::vec3 color;
    };

    struct DebugRay {
        glm::vec3 origin{0.0f};
        glm::vec3 direction{0.0f};
        float length = 0.0f;
        glm::vec3 color{0.0f};
    };

    DebugRenderer() = default;
    ~DebugRenderer() = default;

    DebugRenderer(const DebugRenderer&) = delete;
    DebugRenderer& operator=(const DebugRenderer&) = delete;

    void ensureInitialized();

private:
    bool m_Initialized = false;
    bool m_Enabled = false;

    std::vector<DebugAABB> m_AABBs;
    std::vector<DebugRay> m_Rays;
};

} // namespace PoorCraft
