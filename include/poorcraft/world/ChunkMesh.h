#pragma once

#include "poorcraft/rendering/TextureAtlas.h"
#include "poorcraft/rendering/VertexArray.h"
#include "poorcraft/world/BlockType.h"
#include "poorcraft/world/Chunk.h"
#include "poorcraft/world/ChunkCoord.h"

#include <array>
#include <memory>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace PoorCraft {

class ChunkManager;

enum class MeshFaceDirection : uint8_t {
    POS_X,
    NEG_X,
    POS_Y,
    NEG_Y,
    POS_Z,
    NEG_Z
};

struct BlockVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

class ChunkMesh {
public:
    explicit ChunkMesh(const ChunkCoord& position);
    ChunkMesh(const ChunkMesh&) = delete;
    ChunkMesh& operator=(const ChunkMesh&) = delete;
    ChunkMesh(ChunkMesh&&) noexcept = default;
    ChunkMesh& operator=(ChunkMesh&&) noexcept = default;
    ~ChunkMesh();

    bool generate(Chunk& chunk, ChunkManager& manager, TextureAtlas& atlas);
    void clear();

    [[nodiscard]] bool isEmpty() const;
    [[nodiscard]] std::size_t getVertexCount() const;
    [[nodiscard]] std::size_t getIndexCount() const;
    [[nodiscard]] std::shared_ptr<VertexArray> getVAO() const;
    [[nodiscard]] const ChunkCoord& getPosition() const;

private:
    void greedyMeshFace(Chunk& chunk,
                        ChunkManager& manager,
                        TextureAtlas& atlas,
                        MeshFaceDirection face);

    bool shouldRenderFace(uint16_t blockId, uint16_t neighborId) const;
    std::array<glm::vec2, 4> getBlockUVs(TextureAtlas& atlas,
                                         uint16_t blockId,
                                         MeshFaceDirection face) const;

    uint16_t getBlockFromNeighbors(const Chunk& chunk,
                                   ChunkManager& manager,
                                   int32_t x,
                                   int32_t y,
                                   int32_t z) const;

    static glm::vec3 getNormal(MeshFaceDirection face);
    static glm::vec3 getUAxis(MeshFaceDirection face);
    static glm::vec3 getVAxis(MeshFaceDirection face);
    static BlockFace toBlockFace(MeshFaceDirection face);
    static int axisIndex(MeshFaceDirection face);
    static int uAxisIndex(MeshFaceDirection face);
    static int vAxisIndex(MeshFaceDirection face);
    static bool isPositiveDirection(MeshFaceDirection face);

    ChunkCoord chunkPosition;
    std::vector<BlockVertex> vertices;
    std::vector<uint32_t> indices;
    std::shared_ptr<VertexArray> vao;
};

} // namespace PoorCraft
