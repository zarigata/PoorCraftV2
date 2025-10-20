#pragma once

#include "poorcraft/rendering/Camera.h"
#include "poorcraft/rendering/Shader.h"
#include "poorcraft/rendering/VertexArray.h"

#include <memory>
#include <glm/vec3.hpp>

namespace PoorCraft {

struct SkyColors {
    glm::vec3 skyTopColor;
    glm::vec3 skyHorizonColor;
    glm::vec3 sunColor;
    glm::vec3 moonColor;
};

class SkyRenderer {
public:
    SkyRenderer();
    SkyRenderer(const SkyRenderer&) = delete;
    SkyRenderer& operator=(const SkyRenderer&) = delete;
    SkyRenderer(SkyRenderer&&) = delete;
    SkyRenderer& operator=(SkyRenderer&&) = delete;
    ~SkyRenderer() = default;

    bool initialize();
    void shutdown();
    void render(const Camera& camera, float timeOfDay);

private:
    std::shared_ptr<VertexArray> createSkyDome();
    SkyColors calculateSkyColors(float timeOfDay) const;

    std::shared_ptr<Shader> skyShader;
    std::shared_ptr<VertexArray> skyMesh;
};

} // namespace PoorCraft
