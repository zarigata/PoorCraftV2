#include "poorcraft/rendering/ParticleSystem.h"

#include "poorcraft/core/Config.h"
#include "poorcraft/core/EventBus.h"
#include "poorcraft/core/Logger.h"
#include "poorcraft/modding/ModEvents.h"
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
      particleAtlas(nullptr), maxParticles(10000), blockBreakEventListenerId(0), instanceVBO(0) {}

bool ParticleSystem::initialize() {
    PC_INFO("Initializing ParticleSystem...");

    // Read config
    auto& config = poorcraft::Config::get_instance();
    maxParticles = static_cast<size_t>(config.get_int(poorcraft::Config::RenderingConfig::MAX_PARTICLES_KEY, 10000));
    particles.reserve(maxParticles);
    instanceBuffer.reserve(maxParticles);

    auto& resourceManager = ResourceManager::getInstance();
    particleShader = resourceManager.loadShader("particle", 
        "shaders/particle/particle.vert", "shaders/particle/particle.frag");

    if (!particleShader) {
        PC_ERROR("Failed to load particle shader");
        return false;
    }

    // Load particle atlas
    particleAtlas = std::make_shared<TextureAtlas>(256, TextureFormat::RGBA);
    const std::string particlePath = resourceManager.resolvePath("assets/textures/particles/");
    
    // Try to load a default particle texture (fallback to white if missing)
    if (!particleAtlas->addTextureFromFile("default", particlePath + "default.png")) {
        PC_WARN("No particle textures found, using fallback");
    }
    
    if (!particleAtlas->build()) {
        PC_WARN("Failed to build particle atlas, particles may not render correctly");
    }

    // Create quad VAO for particle billboards with instancing
    particleVAO = std::make_shared<VertexArray>();
    if (!particleVAO->create()) {
        PC_ERROR("Failed to create particle VAO");
        return false;
    }

    // Quad vertices (position and UV)
    const float quadVertices[] = {
        // pos      // uv
        -0.5f, -0.5f, 0.0f, 0.0f,
         0.5f, -0.5f, 1.0f, 0.0f,
         0.5f,  0.5f, 1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f, 1.0f
    };

    const uint32_t quadIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    particleVAO->bind();

    // Per-vertex attributes (quad geometry)
    const std::vector<VertexAttribute> vertexAttributes = {
        {0, 2, VertexAttributeType::FLOAT, false, 4 * sizeof(float), 0},
        {1, 2, VertexAttributeType::FLOAT, false, 4 * sizeof(float), 2 * sizeof(float)}
    };

    particleVAO->addVertexBuffer(quadVertices, sizeof(quadVertices), vertexAttributes, BufferUsage::STATIC_DRAW);
    particleVAO->setIndexBuffer(quadIndices, 6, BufferUsage::STATIC_DRAW);

    // Create instance buffer (will be updated each frame)
    // Reserve space for instance attributes but don't upload data yet
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(InstanceData), nullptr, GL_DYNAMIC_DRAW);

    // Instance attributes (per-particle data)
    // Position (vec3)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, position));
    glVertexAttribDivisor(2, 1);

    // Color (vec4)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, color));
    glVertexAttribDivisor(3, 1);

    // Size (float)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, size));
    glVertexAttribDivisor(4, 1);

    // Rotation (float)
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, rotation));
    glVertexAttribDivisor(5, 1);

    VertexArray::unbind();

    // Register event listener for block break
    auto& eventBus = EventBus::getInstance();
    blockBreakEventListenerId = eventBus.subscribe<BlockBrokenEvent>(
        [this](const BlockBrokenEvent& event) {
            const glm::vec3 pos(static_cast<float>(event.getX()) + 0.5f,
                               static_cast<float>(event.getY()) + 0.5f,
                               static_cast<float>(event.getZ()) + 0.5f);
            spawnBlockBreakParticles(event.getBlockId(), pos);
        }
    );

    PC_INFO("ParticleSystem initialized");
    return true;
}

void ParticleSystem::shutdown() {
    // Unregister event listener
    if (blockBreakEventListenerId != 0) {
        EventBus::getInstance().unsubscribe(blockBreakEventListenerId);
        blockBreakEventListenerId = 0;
    }

    particles.clear();
    emitters.clear();
    instanceBuffer.clear();

    if (instanceVBO != 0) {
        glDeleteBuffers(1, &instanceVBO);
        instanceVBO = 0;
    }

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
    if (particles.empty() || !particleShader || !particleVAO) {
        return;
    }

    sortParticlesByDepth(camera.getPosition());

    // Build instance buffer
    instanceBuffer.clear();
    for (const auto& particle : particles) {
        InstanceData instance;
        instance.position = particle.position;
        instance.color = particle.color;
        instance.size = particle.size;
        instance.rotation = particle.rotation;
        instanceBuffer.push_back(instance);
    }

    if (instanceBuffer.empty()) {
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    particleShader->use();
    particleShader->setMat4("view", camera.getViewMatrix());
    particleShader->setMat4("projection", camera.getProjectionMatrix());
    particleShader->setVec3("cameraRight", camera.getRight());
    particleShader->setVec3("cameraUp", camera.getUp());

    // Bind particle atlas if available
    if (particleAtlas) {
        auto atlasTexture = particleAtlas->getTexture();
        if (atlasTexture) {
            atlasTexture->bind(0);
            particleShader->setInt("particleAtlas", 0);
        }
    }

    particleVAO->bind();

    // Update instance buffer
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, instanceBuffer.size() * sizeof(InstanceData), instanceBuffer.data());

    // Draw all particles with one instanced call
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, static_cast<GLsizei>(instanceBuffer.size()));

    VertexArray::unbind();

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
