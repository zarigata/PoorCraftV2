#pragma once

#include "poorcraft/rendering/Camera.h"
#include "poorcraft/rendering/Shader.h"
#include "poorcraft/rendering/TextureAtlas.h"
#include "poorcraft/world/ChunkCoord.h"

#include <memory>
#include <unordered_map>
#include <glm/vec4.hpp>

namespace PoorCraft {

class ChunkManager;
class VertexArray;

class WaterRenderer {
public:
    WaterRenderer(ChunkManager& chunkManager, TextureAtlas& textureAtlas);
    WaterRenderer(const WaterRenderer&) = delete;
    WaterRenderer& operator=(const WaterRenderer&) = delete;
    WaterRenderer(WaterRenderer&&) = delete;
    WaterRenderer& operator=(WaterRenderer&&) = delete;
    ~WaterRenderer() = default;

    bool initialize();
    void shutdown();
    void render(const Camera& camera, float time);

private:
    void sortWaterChunksByDepth(std::vector<ChunkCoord>& chunks, const glm::vec3& cameraPos);

    ChunkManager& chunkManager;
    TextureAtlas& textureAtlas;
    std::shared_ptr<Shader> waterShader;
    glm::vec4 waterColor;
};

} // namespace PoorCraft
