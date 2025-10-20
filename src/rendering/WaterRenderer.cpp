#include "poorcraft/rendering/WaterRenderer.h"

#include "poorcraft/core/Config.h"
#include "poorcraft/core/Logger.h"
#include "poorcraft/resource/ResourceManager.h"
#include "poorcraft/world/BlockRegistry.h"
#include "poorcraft/world/Chunk.h"
#include "poorcraft/world/ChunkManager.h"

#include <algorithm>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace PoorCraft {

WaterRenderer::WaterRenderer(ChunkManager& chunkManager, TextureAtlas& textureAtlas)
    : chunkManager(chunkManager), textureAtlas(textureAtlas), waterShader(nullptr),
      waterColor(0.2f, 0.4f, 0.8f, 0.7f), waveSpeed(1.0f) {}

bool WaterRenderer::initialize() {
    PC_INFO("Initializing WaterRenderer...");

    auto& resourceManager = ResourceManager::getInstance();
    waterShader = resourceManager.loadShader("water", "shaders/water/water.vert", "shaders/water/water.frag");

    if (!waterShader) {
        PC_ERROR("Failed to load water shader");
        return false;
    }

    // Load water config
    auto& config = poorcraft::Config::get_instance();
    const float transparency = config.get_float(poorcraft::Config::RenderingConfig::WATER_TRANSPARENCY_KEY, 0.7f);
    waterColor.a = transparency;
    waveSpeed = config.get_float(poorcraft::Config::RenderingConfig::WATER_WAVE_SPEED_KEY, 1.0f);

    PC_INFO("WaterRenderer initialized");
    return true;
}

void WaterRenderer::shutdown() {
    if (waterShader) {
        waterShader.reset();
    }
    PC_INFO("WaterRenderer shutdown");
}

void WaterRenderer::render(const Camera& camera, float time, const glm::vec3& sunDirection, 
                           const glm::vec3& sunColor, float ambientStrength) {
    if (!waterShader) {
        return;
    }

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glEnable(GL_DEPTH_TEST);

    waterShader->use();
    waterShader->setMat4("view", camera.getViewMatrix());
    waterShader->setMat4("projection", camera.getProjectionMatrix());
    waterShader->setFloat("time", time * waveSpeed);
    waterShader->setVec4("waterColor", waterColor);

    // Set lighting uniforms from World
    waterShader->setVec3("sunDirection", sunDirection);
    waterShader->setVec3("sunColor", sunColor);
    waterShader->setFloat("ambientStrength", ambientStrength);

    // Bind water texture from atlas
    auto atlasTexture = textureAtlas.getTexture();
    if (atlasTexture) {
        atlasTexture->bind(0);
        waterShader->setInt("blockAtlas", 0);
    }

    // Collect water chunks and sort back-to-front
    std::vector<ChunkCoord> waterChunks;
    const auto& meshes = chunkManager.getMeshes();
    for (const auto& pair : meshes) {
        const ChunkCoord& coord = pair.first;
        const auto& mesh = pair.second;
        
        // Check if chunk has water sub-mesh
        if (mesh && mesh->hasWater()) {
            waterChunks.push_back(coord);
        }
    }

    // Sort water chunks back-to-front
    if (!waterChunks.empty()) {
        sortWaterChunksByDepth(waterChunks, camera.getPosition());
        
        // Render water meshes
        for (const auto& coord : waterChunks) {
            const auto* mesh = chunkManager.getChunkMesh(coord);
            if (!mesh || !mesh->hasWater()) {
                continue;
            }

            auto waterVAO = mesh->getWaterVAO();
            if (!waterVAO) {
                continue;
            }

            // Set model matrix for chunk position
            const glm::vec3 worldPos = coord.toWorldPos();
            glm::mat4 model = glm::translate(glm::mat4(1.0f), worldPos);
            waterShader->setMat4("model", model);

            // Draw water mesh
            waterVAO->bind();
            waterVAO->draw(GL_TRIANGLES, mesh->getWaterIndexCount());
            VertexArray::unbind();
        }
    }

    // Restore GL state
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
