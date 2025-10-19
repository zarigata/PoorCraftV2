#pragma once

#include <array>
#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "poorcraft/entity/PlayerSkin.h"
#include "poorcraft/rendering/VertexArray.h"

namespace PoorCraft {

class HumanoidMesh {
public:
    struct MeshSection {
        uint32_t indexOffset = 0;
        uint32_t indexCount = 0;
    };

    struct MeshData {
        std::shared_ptr<VertexArray> mesh;
        std::array<MeshSection, 6> sections{};
    };

    static MeshData generate(const PlayerSkin* skin);

private:
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    static MeshSection addCube(std::vector<Vertex>& vertices,
                               std::vector<uint32_t>& indices,
                               const glm::vec3& min,
                               const glm::vec3& max,
                               const PlayerSkin::Region& region);
};

} // namespace PoorCraft
