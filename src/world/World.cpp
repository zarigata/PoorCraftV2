#include "poorcraft/world/World.h"

#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

#include "poorcraft/core/Config.h"
#include "poorcraft/core/Logger.h"
#include "poorcraft/core/EventBus.h"
#include "poorcraft/modding/ModEvents.h"
#include "poorcraft/rendering/ParticleSystem.h"
#include "poorcraft/resource/ResourceManager.h"

namespace PoorCraft {

namespace {

constexpr int DEFAULT_ATLAS_SIZE = 1024;

} // namespace

World::World() : chunkManager(std::make_unique<ChunkManager>()), textureAtlas(nullptr), 
    lightingManager(nullptr), skyRenderer(nullptr), waterRenderer(nullptr), 
    timeOfDay(0.5f), dayNightCycleSpeed(1.0f), renderStats() {}

World::~World() {
    shutdown();
}

bool World::initialize(int renderDistance) {
    PC_INFO("Initializing World...");

    BlockRegistry::getInstance().initialize();

    // Load rendering configuration
    auto& config = poorcraft::Config::get_instance();
    timeOfDay = config.get_float(poorcraft::Config::RenderingConfig::STARTING_TIME_KEY, 0.5f);
    
    const bool enableDayNight = config.get_bool(poorcraft::Config::RenderingConfig::ENABLE_DAY_NIGHT_CYCLE_KEY, true);
    if (enableDayNight) {
        dayNightCycleSpeed = config.get_float(poorcraft::Config::RenderingConfig::DAY_NIGHT_CYCLE_SPEED_KEY, 1.0f);
    } else {
        dayNightCycleSpeed = 0.0f; // Disable cycle
    }

    textureAtlas = createBlockTextureAtlas();
    if (!textureAtlas) {
        PC_ERROR("Failed to create block texture atlas.");
        return false;
    }

    chunkManager->initialize();
    chunkManager->setTextureAtlas(textureAtlas.get());

    // Create and initialize LightingManager
    lightingManager = std::make_unique<LightingManager>(*chunkManager);
    lightingManager->initialize();
    chunkManager->setLightingManager(lightingManager.get());

    // Create and initialize SkyRenderer (if enabled)
    const bool enableSky = config.get_bool(poorcraft::Config::RenderingConfig::ENABLE_SKY_KEY, true);
    if (enableSky) {
        skyRenderer = std::make_unique<SkyRenderer>();
        if (!skyRenderer->initialize()) {
            PC_WARN("Failed to initialize SkyRenderer, sky will not be rendered");
            skyRenderer.reset();
        }
    }

    // Create and initialize WaterRenderer
    waterRenderer = std::make_unique<WaterRenderer>(*chunkManager, *textureAtlas);
    if (!waterRenderer->initialize()) {
        PC_WARN("Failed to initialize WaterRenderer, water will not be rendered");
        waterRenderer.reset();
    }

    // Initialize ParticleSystem (if enabled)
    const bool enableParticles = config.get_bool(poorcraft::Config::RenderingConfig::ENABLE_PARTICLES_KEY, true);
    if (enableParticles) {
        if (!ParticleSystem::getInstance().initialize()) {
            PC_WARN("Failed to initialize ParticleSystem, particles will not be rendered");
        }
    }

    PC_INFO("World initialized with render distance " + std::to_string(renderDistance));
    return true;
}

void World::shutdown() {
    PC_INFO("Shutting down World...");

    ParticleSystem::getInstance().shutdown();

    if (waterRenderer) {
        waterRenderer->shutdown();
        waterRenderer.reset();
    }

    if (skyRenderer) {
        skyRenderer->shutdown();
        skyRenderer.reset();
    }

    if (lightingManager) {
        lightingManager.reset();
    }

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

void World::update(const glm::vec3& cameraPosition, int renderDistance, float deltaTime) {
    if (!chunkManager) {
        return;
    }

    // Update time of day (only if cycle is enabled)
    if (dayNightCycleSpeed > 0.0f) {
        timeOfDay += deltaTime * dayNightCycleSpeed / 1200.0f; // 1200 seconds = 20 minutes per day
        if (timeOfDay > 1.0f) {
            timeOfDay -= 1.0f;
        }
    }

    chunkManager->update(cameraPosition, renderDistance);
    
    // Update particle system (if enabled)
    auto& config = poorcraft::Config::get_instance();
    const bool enableParticles = config.get_bool(poorcraft::Config::RenderingConfig::ENABLE_PARTICLES_KEY, true);
    if (enableParticles) {
        ParticleSystem::getInstance().update(deltaTime);
    }
}

void World::render(const Camera& camera, Shader& shader) {
    if (!chunkManager || !textureAtlas) {
        return;
    }

    renderStats = {};
    renderStats.totalChunks = chunkManager->getLoadedChunkCount();

    const glm::mat4 viewProjection = camera.getViewProjectionMatrix();
    Frustum frustum(viewProjection);

    // === PASS 1: Render Sky ===
    if (skyRenderer) {
        skyRenderer->render(camera, timeOfDay);
    }

    // === PASS 2: Render Opaque Chunks ===
    shader.use();
    shader.setMat4("view", camera.getViewMatrix());
    shader.setMat4("projection", camera.getProjectionMatrix());

    // Set lighting uniforms
    auto& config = poorcraft::Config::get_instance();
    const float ambientLevel = config.get_float(poorcraft::Config::RenderingConfig::AMBIENT_LIGHT_LEVEL_KEY, 0.3f);
    
    shader.setVec3("sunDirection", getSunDirection());
    shader.setVec3("sunColor", getSunColor());
    shader.setFloat("ambientStrength", ambientLevel);

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

    // === PASS 3: Render Water ===
    if (waterRenderer) {
        waterRenderer->render(camera, timeOfDay, getSunDirection(), getSunColor(), ambientLevel);
    }

    // === PASS 4: Render Particles ===
    const bool enableParticles = config.get_bool(poorcraft::Config::RenderingConfig::ENABLE_PARTICLES_KEY, true);
    if (enableParticles) {
        ParticleSystem::getInstance().render(camera);
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
        {"flower", basePath + "flower.png"},
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

bool World::setBlockAt(int32_t worldX, int32_t worldY, int32_t worldZ, uint16_t blockId, uint32_t playerId) {
    if (!chunkManager) {
        return false;
    }
    
    // Convert world coordinates to chunk coordinates and local block coordinates
    const int32_t chunkX = worldX < 0 ? (worldX - Chunk::CHUNK_SIZE_X + 1) / Chunk::CHUNK_SIZE_X : worldX / Chunk::CHUNK_SIZE_X;
    const int32_t chunkZ = worldZ < 0 ? (worldZ - Chunk::CHUNK_SIZE_Z + 1) / Chunk::CHUNK_SIZE_Z : worldZ / Chunk::CHUNK_SIZE_Z;
    const int32_t localX = worldX - chunkX * Chunk::CHUNK_SIZE_X;
    const int32_t localZ = worldZ - chunkZ * Chunk::CHUNK_SIZE_Z;
    
    ChunkCoord coord(chunkX, chunkZ);
    Chunk* chunk = chunkManager->getChunk(coord);
    if (!chunk) {
        return false;
    }
    
    // Get previous block ID
    uint16_t previousBlockId = chunk->getBlock(localX, worldY, localZ);
    
    // Set the block
    chunk->setBlock(localX, worldY, localZ, blockId);
    
    // Update lighting for the affected chunk and neighbors
    if (lightingManager) {
        lightingManager->updateChunkLighting(coord);
        // Also update neighbors if on chunk boundary
        if (localX == 0 || localX == Chunk::CHUNK_SIZE_X - 1 ||
            localZ == 0 || localZ == Chunk::CHUNK_SIZE_Z - 1) {
            const ChunkCoord neighbors[4] = {
                {coord.x + 1, coord.z},
                {coord.x - 1, coord.z},
                {coord.x, coord.z + 1},
                {coord.x, coord.z - 1}
            };
            for (const auto& neighborCoord : neighbors) {
                if (chunkManager->hasChunk(neighborCoord)) {
                    lightingManager->updateChunkLighting(neighborCoord);
                }
            }
        }
    }
    
    // Publish appropriate event
    if (blockId != 0 && previousBlockId == 0) {
        // Block placed
        BlockPlacedEvent event(worldX, worldY, worldZ, blockId, static_cast<EntityID>(playerId), previousBlockId);
        EventBus::getInstance().publish(event);
    } else if (blockId == 0 && previousBlockId != 0) {
        // Block broken
        BlockBrokenEvent event(worldX, worldY, worldZ, previousBlockId, static_cast<EntityID>(playerId));
        EventBus::getInstance().publish(event);
    } else if (blockId != previousBlockId && blockId != 0 && previousBlockId != 0) {
        // Block replaced (publish both events)
        BlockBrokenEvent breakEvent(worldX, worldY, worldZ, previousBlockId, static_cast<EntityID>(playerId));
        EventBus::getInstance().publish(breakEvent);
        BlockPlacedEvent placeEvent(worldX, worldY, worldZ, blockId, static_cast<EntityID>(playerId), previousBlockId);
        EventBus::getInstance().publish(placeEvent);
    }
    
    return true;
}

uint16_t World::getBlockAt(int32_t worldX, int32_t worldY, int32_t worldZ) const {
    if (!chunkManager) {
        return 0;
    }
    
    // Convert world coordinates to chunk coordinates and local block coordinates
    const int32_t chunkX = worldX < 0 ? (worldX - Chunk::CHUNK_SIZE_X + 1) / Chunk::CHUNK_SIZE_X : worldX / Chunk::CHUNK_SIZE_X;
    const int32_t chunkZ = worldZ < 0 ? (worldZ - Chunk::CHUNK_SIZE_Z + 1) / Chunk::CHUNK_SIZE_Z : worldZ / Chunk::CHUNK_SIZE_Z;
    const int32_t localX = worldX - chunkX * Chunk::CHUNK_SIZE_X;
    const int32_t localZ = worldZ - chunkZ * Chunk::CHUNK_SIZE_Z;
    
    ChunkCoord coord(chunkX, chunkZ);
    Chunk* chunk = chunkManager->getChunk(coord);
    if (!chunk) {
        return 0;
    }
    
    return chunk->getBlock(localX, worldY, localZ);
}

float World::getTimeOfDay() const {
    return timeOfDay;
}

void World::setTimeOfDay(float time) {
    timeOfDay = std::fmod(time, 1.0f);
    if (timeOfDay < 0.0f) {
        timeOfDay += 1.0f;
    }
}

glm::vec3 World::getSunDirection() const {
    const float angle = timeOfDay * 2.0f * static_cast<float>(M_PI) - static_cast<float>(M_PI) / 2.0f;
    return glm::vec3(0.0f, std::sin(angle), std::cos(angle));
}

glm::vec3 World::getSunColor() const {
    const glm::vec3 sunDir = getSunDirection();
    const float sunHeight = sunDir.y;
    
    if (sunHeight > 0.0f) {
        // Day time - yellow-orange sun
        return glm::vec3(1.0f, 0.9f, 0.7f);
    } else {
        // Night time - dark blue
        const float t = std::max(0.0f, sunHeight + 0.2f) / 0.2f;
        return glm::mix(glm::vec3(0.1f, 0.1f, 0.2f), glm::vec3(1.0f, 0.9f, 0.7f), t);
    }
}

glm::vec3 World::getMoonDirection() const {
    return -getSunDirection();
}

glm::vec3 World::getSkyColor() const {
    const glm::vec3 sunDir = getSunDirection();
    const float sunHeight = sunDir.y;
    
    const glm::vec3 daySky(0.5f, 0.7f, 1.0f);
    const glm::vec3 nightSky(0.05f, 0.05f, 0.1f);
    
    const float t = (sunHeight + 1.0f) * 0.5f;
    return glm::mix(nightSky, daySky, t);
}

} // namespace PoorCraft
