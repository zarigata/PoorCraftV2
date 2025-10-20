#include "poorcraft/rendering/WaterRenderer.h"

#include "poorcraft/core/Logger.h"
#include "poorcraft/resource/ResourceManager.h"
#include "poorcraft/world/BlockRegistry.h"
#include "poorcraft/world/Chunk.h"
#include "poorcraft/world/ChunkManager.h"

#include <algorithm>
#include <glad/glad.h>

namespace PoorCraft {

WaterRenderer::WaterRenderer(ChunkManager& chunkManager, TextureAtlas& textureAtlas)
    : chunkManager(chunkManager), textureAtlas(textureAtlas), waterShader(nullptr),
      waterColor(0.2f, 0.4f, 0.8f, 0.7f) {}

bool WaterRenderer::initialize() {
    PC_INFO("Initializing WaterRenderer...");

    auto& resourceManager = ResourceManager::getInstance();
    waterShader = resourceManager.loadShader("water", "shaders/water/water.vert", "shaders/water/water.frag");

    if (!waterShader) {
        PC_ERROR("Failed to load water shader");
        return false;
    }

    PC_INFO("WaterRenderer initialized");
    return true;
}

void WaterRenderer::shutdown() {
    if (waterShader) {
        waterShader.reset();
    }
    PC_INFO("WaterRenderer shutdown");
}

void WaterRenderer::render(const Camera& camera, float time) {
    if (!waterShader) {
        return;
    }

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    waterShader->use();
    waterShader->setMat4("view", camera.getViewMatrix());
    waterShader->setMat4("projection", camera.getProjectionMatrix());
    waterShader->setFloat("time", time);
    waterShader->setVec4("waterColor", waterColor);

    // Bind water texture from atlas
    auto atlasTexture = textureAtlas.getTexture();
    if (atlasTexture) {
        atlasTexture->bind(0);
        waterShader->setInt("blockAtlas", 0);
    }

    // Render water chunks (simplified - actual implementation would filter water blocks)
    // This is a placeholder for the full water rendering implementation

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void WaterRenderer::sortWaterChunksByDepth(std::vector<ChunkCoord>& chunks, const glm::vec3& cameraPos) {
    std::sort(chunks.begin(), chunks.end(), [&cameraPos](const ChunkCoord& a, const ChunkCoord& b) {
        const glm::vec3 aPos = a.toWorldPos() + glm::vec3(8.0f, 128.0f, 8.0f);
        const glm::vec3 bPos = b.toWorldPos() + glm::vec3(8.0f, 128.0f, 8.0f);
        const float aDist = glm::distance(cameraPos, aPos);
        const float bDist = glm::distance(cameraPos, bPos);
        return aDist > bDist; // Farthest first
    });
}

} // namespace PoorCraft
