#pragma once

#include <glm/glm.hpp>

#include "poorcraft/world/BlockType.h"

namespace PoorCraft {

struct RaycastHit {
    bool hit = false;
    glm::vec3 position{0.0f};
    glm::ivec3 blockPos{0};
    glm::vec3 normal{0.0f};
    BlockFace face = BlockFace::FRONT;
    float distance = 0.0f;
};

} // namespace PoorCraft
