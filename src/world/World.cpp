#include "poorcraft/world/World.h"

#include <glm/gtc/matrix_transform.hpp>

#include "poorcraft/core/Logger.h"
#include "poorcraft/resource/ResourceManager.h"

namespace PoorCraft {

namespace {

constexpr int DEFAULT_ATLAS_SIZE = 1024;

} // namespace

World::World() : chunkManager(std::make_unique<ChunkManager>()), textureAtlas(nullptr), renderStats() {}

World::~World() {
    shutdown();
}

bool World::initialize(int renderDistance) {
    PC_INFO("Initializing World...");

    BlockRegistry::getInstance().initialize();

    textureAtlas = createBlockTextureAtlas();
    if (!textureAtlas) {
        PC_ERROR("Failed to create block texture atlas.");
        return false;
    }

    chunkManager->initialize();
    chunkManager->setTextureAtlas(textureAtlas.get());

    PC_INFO("World initialized with render distance " + std::to_string(renderDistance));
    return true;
}

void World::shutdown() {
    PC_INFO("Shutting down World...");

    if (chunkManager) {
        chunkManager->shutdown();
        chunkManager.reset();
    }

    if (textureAtlas) {
        textureAtlas.reset();
    }

    renderStats = {};

    PC_INFO("World shutdown complete.");
}

void World::update(const glm::vec3& cameraPosition, int renderDistance) {
    if (!chunkManager) {
        return;
    }

    chunkManager->update(cameraPosition, renderDistance);
}

void World::render(const Camera& camera, Shader& shader) {
    if (!chunkManager || !textureAtlas) {
        return;
    }

    renderStats = {};
    renderStats.totalChunks = chunkManager->getLoadedChunkCount();

    const glm::mat4 viewProjection = camera.getViewProjectionMatrix();
    Frustum frustum(viewProjection);

    shader.setMat4("view", camera.getViewMatrix());
    shader.setMat4("projection", camera.getProjectionMatrix());

    auto atlasTexture = textureAtlas->getTexture();
    if (atlasTexture) {
        atlasTexture->bind(0);
        shader.setInt("blockAtlas", 0);
    }

    for (const auto& pair : chunkManager->getMeshes()) {
        const ChunkCoord& coord = pair.first;
        const auto& meshPtr = pair.second;
        if (!meshPtr || meshPtr->isEmpty()) {
            ++renderStats.chunksCulled;
            continue;
        }

        const glm::vec3 worldPos = coord.toWorldPos();
        const AABB bounds{worldPos, worldPos + glm::vec3(static_cast<float>(Chunk::CHUNK_SIZE_X),
                                                         static_cast<float>(Chunk::CHUNK_SIZE_Y),
                                                         static_cast<float>(Chunk::CHUNK_SIZE_Z))};

        if (!frustum.containsAABB(bounds)) {
            ++renderStats.chunksCulled;
            continue;
        }

        glm::mat4 model = glm::translate(glm::mat4(1.0f), worldPos);
        shader.setMat4("model", model);

        auto vao = meshPtr->getVAO();
        if (!vao) {
            ++renderStats.chunksCulled;
            continue;
        }

        vao->bind();
        vao->draw(GL_TRIANGLES, meshPtr->getIndexCount());
        VertexArray::unbind();

        renderStats.chunksRendered += 1;
        renderStats.verticesRendered += meshPtr->getVertexCount();
    }
}

ChunkManager& World::getChunkManager() {
    return *chunkManager;
}

BlockRegistry& World::getBlockRegistry() {
    return BlockRegistry::getInstance();
}

TextureAtlas& World::getTextureAtlas() {
    return *textureAtlas;
}

const WorldRenderStats& World::getRenderStats() const {
    return renderStats;
}

std::unique_ptr<TextureAtlas> World::createBlockTextureAtlas() {
    auto atlas = std::make_unique<TextureAtlas>(DEFAULT_ATLAS_SIZE, TextureFormat::RGBA);

    auto& resourceManager = ResourceManager::getInstance();
    const std::string basePath = resourceManager.resolvePath("assets/textures/blocks/");

    const std::vector<std::pair<std::string, std::string>> textures = {
        {"stone", basePath + "stone.png"},
        {"dirt", basePath + "dirt.png"},
        {"grass_top", basePath + "grass_top.png"},
        {"grass_side", basePath + "grass_side.png"},
        {"sand", basePath + "sand.png"},
        {"water", basePath + "water.png"},
        {"snow", basePath + "snow.png"},
        {"ice", basePath + "ice.png"},
        {"grass_side_snowy", basePath + "grass_side_snowy.png"},
        {"oak_log_top", basePath + "oak_log_top.png"},
        {"oak_log_side", basePath + "oak_log_side.png"},
        {"oak_leaves", basePath + "oak_leaves.png"},
        {"jungle_log_top", basePath + "jungle_log_top.png"},
        {"jungle_log_side", basePath + "jungle_log_side.png"},
        {"jungle_leaves", basePath + "jungle_leaves.png"},
        {"spruce_log_top", basePath + "spruce_log_top.png"},
        {"spruce_log_side", basePath + "spruce_log_side.png"},
        {"spruce_leaves", basePath + "spruce_leaves.png"},
        {"cactus_top", basePath + "cactus_top.png"},
        {"cactus_side", basePath + "cactus_side.png"},
        {"cactus_bottom", basePath + "cactus_bottom.png"},
        {"sandstone", basePath + "sandstone.png"},
        {"bedrock", basePath + "bedrock.png"},
        {"coal_ore", basePath + "coal_ore.png"},
        {"iron_ore", basePath + "iron_ore.png"},
        {"gold_ore", basePath + "gold_ore.png"},
        {"diamond_ore", basePath + "diamond_ore.png"},
        {"tall_grass", basePath + "tall_grass.png"},
        {"vines", basePath + "vines.png"},
        {"lava", basePath + "lava.png"},
    };

    for (const auto& [name, path] : textures) {
        if (!atlas->addTextureFromFile(name, path)) {
            PC_WARN("Missing block texture at " + path + ", using fallback UVs.");
        }
    }

    if (!atlas->build()) {
        PC_ERROR("Failed to build block texture atlas.");
        return nullptr;
    }

    return atlas;
}

} // namespace PoorCraft
