#include "poorcraft/rendering/SkyRenderer.h"

#include "poorcraft/core/Logger.h"
#include "poorcraft/resource/ResourceManager.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace PoorCraft {

SkyRenderer::SkyRenderer() : skyShader(nullptr), skyMesh(nullptr) {}

bool SkyRenderer::initialize() {
    PC_INFO("Initializing SkyRenderer...");

    auto& resourceManager = ResourceManager::getInstance();
    skyShader = resourceManager.loadShader("sky", "shaders/sky/sky.vert", "shaders/sky/sky.frag");

    if (!skyShader) {
        PC_ERROR("Failed to load sky shader");
        return false;
    }

    skyMesh = createSkyDome();
    if (!skyMesh) {
        PC_ERROR("Failed to create sky dome mesh");
        return false;
    }

    PC_INFO("SkyRenderer initialized");
    return true;
}

void SkyRenderer::shutdown() {
    if (skyShader) {
        skyShader.reset();
    }

    if (skyMesh) {
        skyMesh->destroy();
        skyMesh.reset();
    }

    PC_INFO("SkyRenderer shutdown");
}

void SkyRenderer::render(const Camera& camera, float timeOfDay) {
    if (!skyShader || !skyMesh) {
        return;
    }

    glDepthMask(GL_FALSE);

    skyShader->use();

    // Remove translation from view matrix for infinite distance
    glm::mat4 view = camera.getViewMatrix();
    view[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    skyShader->setMat4("view", view);
    skyShader->setMat4("projection", camera.getProjectionMatrix());

    // Calculate and set sky colors
    SkyColors colors = calculateSkyColors(timeOfDay);
    skyShader->setVec3("skyTopColor", colors.skyTopColor);
    skyShader->setVec3("skyHorizonColor", colors.skyHorizonColor);
    skyShader->setVec3("sunColor", colors.sunColor);
    skyShader->setVec3("moonColor", colors.moonColor);
    skyShader->setFloat("timeOfDay", timeOfDay);

    // Calculate sun and moon directions
    const float angle = timeOfDay * 2.0f * static_cast<float>(M_PI) - static_cast<float>(M_PI) / 2.0f;
    glm::vec3 sunDirection(0.0f, std::sin(angle), std::cos(angle));
    glm::vec3 moonDirection = -sunDirection;

    skyShader->setVec3("sunDirection", sunDirection);
    skyShader->setVec3("moonDirection", moonDirection);

    skyMesh->bind();
    skyMesh->draw(GL_TRIANGLES, 36); // 6 faces * 2 triangles * 3 vertices
    VertexArray::unbind();

    glDepthMask(GL_TRUE);
}

std::shared_ptr<VertexArray> SkyRenderer::createSkyDome() {
    // Create inverted cube for sky
    const float size = 500.0f;
    const float vertices[] = {
        // Positions
        -size, -size, -size,
         size, -size, -size,
         size,  size, -size,
         size,  size, -size,
        -size,  size, -size,
        -size, -size, -size,

        -size, -size,  size,
         size, -size,  size,
         size,  size,  size,
         size,  size,  size,
        -size,  size,  size,
        -size, -size,  size,

        -size,  size,  size,
        -size,  size, -size,
        -size, -size, -size,
        -size, -size, -size,
        -size, -size,  size,
        -size,  size,  size,

         size,  size,  size,
         size,  size, -size,
         size, -size, -size,
         size, -size, -size,
         size, -size,  size,
         size,  size,  size,

        -size, -size, -size,
         size, -size, -size,
         size, -size,  size,
         size, -size,  size,
        -size, -size,  size,
        -size, -size, -size,

        -size,  size, -size,
         size,  size, -size,
         size,  size,  size,
         size,  size,  size,
        -size,  size,  size,
        -size,  size, -size
    };

    auto vao = std::make_shared<VertexArray>();
    if (!vao->create()) {
        return nullptr;
    }

    vao->bind();

    const std::vector<VertexAttribute> attributes = {
        {0, 3, VertexAttributeType::FLOAT, false, 3 * sizeof(float), 0}
    };

    vao->addVertexBuffer(vertices, sizeof(vertices), attributes, BufferUsage::STATIC_DRAW);

    VertexArray::unbind();

    return vao;
}

SkyColors SkyRenderer::calculateSkyColors(float timeOfDay) const {
    const float angle = timeOfDay * 2.0f * static_cast<float>(M_PI) - static_cast<float>(M_PI) / 2.0f;
    const float sunHeight = std::sin(angle);

    SkyColors colors;

    if (sunHeight > 0.0f) {
        // Day time
        colors.skyTopColor = glm::vec3(0.5f, 0.7f, 1.0f);
        colors.skyHorizonColor = glm::vec3(0.8f, 0.9f, 1.0f);
        colors.sunColor = glm::vec3(1.0f, 0.9f, 0.7f);
        colors.moonColor = glm::vec3(0.0f);
    } else {
        // Night time
        colors.skyTopColor = glm::vec3(0.05f, 0.05f, 0.1f);
        colors.skyHorizonColor = glm::vec3(0.1f, 0.1f, 0.15f);
        colors.sunColor = glm::vec3(0.0f);
        colors.moonColor = glm::vec3(0.8f, 0.8f, 1.0f);
    }

    // Smooth transition
    const float t = (sunHeight + 1.0f) * 0.5f;
    const glm::vec3 daySkyTop(0.5f, 0.7f, 1.0f);
    const glm::vec3 nightSkyTop(0.05f, 0.05f, 0.1f);
    colors.skyTopColor = glm::mix(nightSkyTop, daySkyTop, t);

    const glm::vec3 daySkyHorizon(0.8f, 0.9f, 1.0f);
    const glm::vec3 nightSkyHorizon(0.1f, 0.1f, 0.15f);
    colors.skyHorizonColor = glm::mix(nightSkyHorizon, daySkyHorizon, t);

    return colors;
}

} // namespace PoorCraft
