#pragma once

#include <memory>

#include "poorcraft/entity/EntityManager.h"

namespace PoorCraft {

class AnimationSystem {
public:
    explicit AnimationSystem(EntityManager& manager);

    void update(float deltaTime);

private:
    void updateEntity(Entity& entity, float deltaTime);

    EntityManager& m_Manager;
};

} // namespace PoorCraft
