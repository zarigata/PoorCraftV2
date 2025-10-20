#pragma once

#include "poorcraft/rendering/Camera.h"
#include "poorcraft/rendering/Shader.h"
#include "poorcraft/rendering/TextureAtlas.h"
#include "poorcraft/rendering/VertexArray.h"

#include <memory>
#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace PoorCraft {

enum class ParticleType : uint8_t {
    BLOCK_BREAK,
    EXPLOSION,
    SMOKE,
    WATER_SPLASH,
    FIRE,
    MAGIC
};

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec4 color;
    float size;
    float lifetime;
    float maxLifetime;
    int spriteIndex;
    float rotation;
    float rotationSpeed;
};

struct ParticleEmitter {
    glm::vec3 position;
    float emitRate;
    ParticleType particleType;
    float lifetime;
    bool active;
};

class ParticleSystem {
public:
    static ParticleSystem& getInstance();

    ParticleSystem(const ParticleSystem&) = delete;
    ParticleSystem& operator=(const ParticleSystem&) = delete;

    bool initialize();
    void shutdown();
    void update(float deltaTime);
    void render(const Camera& camera);

    void spawnParticle(ParticleType type,
                       const glm::vec3& position,
                       const glm::vec3& velocity,
                       const glm::vec4& color,
                       float size,
                       float lifetime);

    void spawnBlockBreakParticles(uint16_t blockId, const glm::vec3& position);
    void spawnExplosion(const glm::vec3& position, float radius, int particleCount);
    size_t createEmitter(ParticleType type, const glm::vec3& position, float emitRate, float lifetime);
    void removeEmitter(size_t emitterId);

private:
    ParticleSystem();
    ~ParticleSystem() = default;

    void updateParticle(Particle& particle, float deltaTime);
    void sortParticlesByDepth(const glm::vec3& cameraPos);

    std::vector<Particle> particles;
    std::vector<ParticleEmitter> emitters;
    std::shared_ptr<Shader> particleShader;
    std::shared_ptr<VertexArray> particleVAO;
    std::shared_ptr<TextureAtlas> particleAtlas;
    size_t maxParticles;
};

} // namespace PoorCraft
