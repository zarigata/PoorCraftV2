#pragma once

#include "poorcraft/rendering/Camera.h"
#include "poorcraft/rendering/Shader.h"
#include "poorcraft/rendering/TextureAtlas.h"
#include "poorcraft/world/BlockRegistry.h"
#include "poorcraft/world/ChunkManager.h"
#include "poorcraft/world/Frustum.h"

#include <memory>

namespace PoorCraft {

struct WorldRenderStats {
    std::size_t chunksRendered = 0;
    std::size_t chunksCulled = 0;
    std::size_t totalChunks = 0;
    std::size_t verticesRendered = 0;
};

class World {
public:
    World();
    ~World();

    bool initialize(int renderDistance);
    void shutdown();

    void update(const glm::vec3& cameraPosition, int renderDistance);
    void render(const Camera& camera, Shader& shader);

    ChunkManager& getChunkManager();
    BlockRegistry& getBlockRegistry();
    TextureAtlas& getTextureAtlas();

    [[nodiscard]] const WorldRenderStats& getRenderStats() const;

private:
    std::unique_ptr<TextureAtlas> createBlockTextureAtlas();

    std::unique_ptr<ChunkManager> chunkManager;
    std::unique_ptr<TextureAtlas> textureAtlas;

    WorldRenderStats renderStats;
};

} // namespace PoorCraft
