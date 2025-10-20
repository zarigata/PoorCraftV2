#include "poorcraft/rendering/ParticleSystem.h"

#include "poorcraft/core/Logger.h"
#include "poorcraft/resource/ResourceManager.h"
#include "poorcraft/world/BlockRegistry.h"

#include <algorithm>
#include <random>
#include <glad/glad.h>

namespace PoorCraft {

ParticleSystem& ParticleSystem::getInstance() {
    static ParticleSystem instance;
    return instance;
}

ParticleSystem::ParticleSystem()
    : particles(), emitters(), particleShader(nullptr), particleVAO(nullptr),
      particleAtlas(nullptr), maxParticles(10000) {}

bool ParticleSystem::initialize() {
    PC_INFO("Initializing ParticleSystem...");

    auto& resourceManager = ResourceManager::getInstance();
    particleShader = resourceManager.loadShader("particle", 
        "shaders/particle/particle.vert", "shaders/particle/particle.frag");

    if (!particleShader) {
        PC_ERROR("Failed to load particle shader");
        return false;
    }

    particles.reserve(maxParticles);

    PC_INFO("ParticleSystem initialized");
    return true;
}

void ParticleSystem::shutdown() {
    particles.clear();
    emitters.clear();

    if (particleShader) {
        particleShader.reset();
    }

    if (particleVAO) {
        particleVAO.reset();
    }

    if (particleAtlas) {
        particleAtlas.reset();
    }

    PC_INFO("ParticleSystem shutdown");
}

void ParticleSystem::update(float deltaTime) {
    // Update particles
    for (auto it = particles.begin(); it != particles.end();) {
        updateParticle(*it, deltaTime);
        if (it->lifetime <= 0.0f) {
            it = particles.erase(it);
        } else {
            ++it;
        }
    }

    // Update emitters
    for (auto& emitter : emitters) {
        if (!emitter.active) {
            continue;
        }

        emitter.lifetime -= deltaTime;
        if (emitter.lifetime <= 0.0f) {
            emitter.active = false;
        }
    }

    // Remove inactive emitters
    emitters.erase(std::remove_if(emitters.begin(), emitters.end(),
                                   [](const ParticleEmitter& e) { return !e.active; }),
                   emitters.end());
}

void ParticleSystem::render(const Camera& camera) {
    if (particles.empty() || !particleShader) {
        return;
    }

    sortParticlesByDepth(camera.getPosition());

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    particleShader->use();
    particleShader->setMat4("view", camera.getViewMatrix());
    particleShader->setMat4("projection", camera.getProjectionMatrix());
    particleShader->setVec3("cameraRight", camera.getRight());
    particleShader->setVec3("cameraUp", camera.getUp());

    // Render particles (simplified - actual implementation would use instanced rendering)

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void ParticleSystem::spawnParticle(ParticleType type,
                                   const glm::vec3& position,
                                   const glm::vec3& velocity,
                                   const glm::vec4& color,
                                   float size,
                                   float lifetime) {
    if (particles.size() >= maxParticles) {
        return;
    }

    Particle particle;
    particle.position = position;
    particle.velocity = velocity;
    particle.color = color;
    particle.size = size;
    particle.lifetime = lifetime;
    particle.maxLifetime = lifetime;
    particle.spriteIndex = 0;
    particle.rotation = 0.0f;
    particle.rotationSpeed = 0.0f;

    particles.push_back(particle);
}

void ParticleSystem::spawnBlockBreakParticles(uint16_t blockId, const glm::vec3& position) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> velDist(-2.0f, 2.0f);
    static std::uniform_real_distribution<float> offsetDist(-0.5f, 0.5f);
    static std::uniform_real_distribution<float> sizeDist(0.1f, 0.2f);
    static std::uniform_real_distribution<float> lifetimeDist(0.5f, 1.0f);

    for (int i = 0; i < 12; ++i) {
        glm::vec3 vel(velDist(gen), velDist(gen) + 2.0f, velDist(gen));
        glm::vec3 offset(offsetDist(gen), offsetDist(gen), offsetDist(gen));
        
        spawnParticle(ParticleType::BLOCK_BREAK,
                      position + offset,
                      vel,
                      glm::vec4(1.0f),
                      sizeDist(gen),
                      lifetimeDist(gen));
    }

    PC_DEBUG("Spawned block break particles at " + std::to_string(position.x) + ", " +
             std::to_string(position.y) + ", " + std::to_string(position.z));
}

void ParticleSystem::spawnExplosion(const glm::vec3& position, float radius, int particleCount) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
    static std::uniform_real_distribution<float> speedDist(2.0f, 5.0f);

    for (int i = 0; i < particleCount; ++i) {
        float angle = angleDist(gen);
        float speed = speedDist(gen);
        glm::vec3 vel(std::cos(angle) * speed, std::sin(angle) * speed, std::sin(angle) * speed);

        spawnParticle(ParticleType::EXPLOSION,
                      position,
                      vel,
                      glm::vec4(1.0f, 0.5f, 0.0f, 1.0f),
                      0.5f,
                      1.5f);
    }

    PC_DEBUG("Spawned explosion at " + std::to_string(position.x) + ", " +
             std::to_string(position.y) + ", " + std::to_string(position.z));
}

size_t ParticleSystem::createEmitter(ParticleType type,
                                     const glm::vec3& position,
                                     float emitRate,
                                     float lifetime) {
    ParticleEmitter emitter;
    emitter.position = position;
    emitter.emitRate = emitRate;
    emitter.particleType = type;
    emitter.lifetime = lifetime;
    emitter.active = true;

    emitters.push_back(emitter);
    return emitters.size() - 1;
}

void ParticleSystem::removeEmitter(size_t emitterId) {
    if (emitterId < emitters.size()) {
        emitters[emitterId].active = false;
    }
}

void ParticleSystem::updateParticle(Particle& particle, float deltaTime) {
    particle.position += particle.velocity * deltaTime;
    particle.velocity.y -= 9.8f * deltaTime; // Gravity
    particle.rotation += particle.rotationSpeed * deltaTime;
    particle.lifetime -= deltaTime;

    // Fade out over lifetime
    float lifetimeRatio = particle.lifetime / particle.maxLifetime;
    particle.color.a = lifetimeRatio;
}

void ParticleSystem::sortParticlesByDepth(const glm::vec3& cameraPos) {
    std::sort(particles.begin(), particles.end(), [&cameraPos](const Particle& a, const Particle& b) {
        float aDist = glm::distance(cameraPos, a.position);
        float bDist = glm::distance(cameraPos, b.position);
        return aDist > bDist; // Farthest first
    });
}

} // namespace PoorCraft
