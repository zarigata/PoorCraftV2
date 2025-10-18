#include "poorcraft/world/ChunkMesh.h"

#include <algorithm>
#include <array>

#include "poorcraft/core/Logger.h"
#include "poorcraft/world/BlockRegistry.h"
#include "poorcraft/world/ChunkManager.h"

namespace PoorCraft {

ChunkMesh::ChunkMesh(const ChunkCoord& position)
    : chunkPosition(position), vertices(), indices(), vao(nullptr) {}

ChunkMesh::~ChunkMesh() {
    clear();
}

bool ChunkMesh::generate(Chunk& chunk, ChunkManager& manager, TextureAtlas& atlas) {
    clear();

    if (chunk.isEmpty()) {
        return true;
    }

    vertices.reserve(4096);
    indices.reserve(6144);

    for (MeshFaceDirection face : {MeshFaceDirection::POS_X,
                                   MeshFaceDirection::NEG_X,
                                   MeshFaceDirection::POS_Y,
                                   MeshFaceDirection::NEG_Y,
                                   MeshFaceDirection::POS_Z,
                                   MeshFaceDirection::NEG_Z}) {
        greedyMeshFace(chunk, manager, atlas, face);
    }

    if (vertices.empty()) {
        return true;
    }

    vao = std::make_shared<VertexArray>();
    if (!vao->create()) {
        PC_ERROR("Failed to create vertex array for chunk mesh at " + chunkPosition.toString());
        clear();
        return false;
    }

    vao->bind();

    const std::vector<VertexAttribute> attributes = {
        {0, 3, VertexAttributeType::FLOAT, false, sizeof(BlockVertex), offsetof(BlockVertex, position)},
        {1, 3, VertexAttributeType::FLOAT, false, sizeof(BlockVertex), offsetof(BlockVertex, normal)},
        {2, 2, VertexAttributeType::FLOAT, false, sizeof(BlockVertex), offsetof(BlockVertex, uv)},
    };

    vao->addVertexBuffer(vertices.data(), vertices.size() * sizeof(BlockVertex), attributes, BufferUsage::STATIC_DRAW);
    vao->setIndexBuffer(indices.data(), indices.size(), BufferUsage::STATIC_DRAW);

    VertexArray::unbind();

    PC_DEBUG("Generated chunk mesh at " + chunkPosition.toString() +
             " | Vertices: " + std::to_string(vertices.size()) +
             " | Indices: " + std::to_string(indices.size()));

    return true;
}

void ChunkMesh::clear() {
    vertices.clear();
    indices.clear();

    if (vao) {
        vao->destroy();
        vao.reset();
    }
}

bool ChunkMesh::isEmpty() const {
    return vertices.empty();
}

std::size_t ChunkMesh::getVertexCount() const {
    return vertices.size();
}

std::size_t ChunkMesh::getIndexCount() const {
    return indices.size();
}

std::shared_ptr<VertexArray> ChunkMesh::getVAO() const {
    return vao;
}

const ChunkCoord& ChunkMesh::getPosition() const {
    return chunkPosition;
}

void ChunkMesh::greedyMeshFace(Chunk& chunk,
                               ChunkManager& manager,
                               TextureAtlas& atlas,
                               MeshFaceDirection face) {
    constexpr int dims[3] = {Chunk::CHUNK_SIZE_X, Chunk::CHUNK_SIZE_Y, Chunk::CHUNK_SIZE_Z};

    const int axis = axisIndex(face);
    const int uAxis = uAxisIndex(face);
    const int vAxis = vAxisIndex(face);

    const int axisLimit = dims[axis];
    const int uLimit = dims[uAxis];
    const int vLimit = dims[vAxis];

    const bool positive = isPositiveDirection(face);

    std::vector<uint16_t> mask(static_cast<std::size_t>(uLimit * vLimit), 0);

    for (int k = 0; k < axisLimit; ++k) {
        std::fill(mask.begin(), mask.end(), 0);

        for (int v = 0; v < vLimit; ++v) {
            for (int u = 0; u < uLimit; ++u) {
                int coords[3] = {0, 0, 0};
                coords[axis] = k;
                coords[uAxis] = u;
                coords[vAxis] = v;

                const uint16_t blockId = chunk.getBlock(coords[0], coords[1], coords[2]);
                if (blockId == 0) {
                    continue;
                }

                int neighborCoords[3] = {coords[0], coords[1], coords[2]};
                neighborCoords[axis] += positive ? 1 : -1;

                const uint16_t neighborId = getBlockFromNeighbors(chunk,
                                                                    manager,
                                                                    neighborCoords[0],
                                                                    neighborCoords[1],
                                                                    neighborCoords[2]);

                if (shouldRenderFace(blockId, neighborId)) {
                    mask[u + v * uLimit] = blockId;
                }
            }
        }

        for (int v = 0; v < vLimit; ++v) {
            for (int u = 0; u < uLimit;) {
                const uint16_t blockId = mask[u + v * uLimit];
                if (blockId == 0) {
                    ++u;
                    continue;
                }

                int width = 1;
                while (u + width < uLimit && mask[u + width + v * uLimit] == blockId) {
                    ++width;
                }

                int height = 1;
                bool extend = true;
                while (extend && (v + height) < vLimit) {
                    for (int w = 0; w < width; ++w) {
                        if (mask[u + w + (v + height) * uLimit] != blockId) {
                            extend = false;
                            break;
                        }
                    }
                    if (extend) {
                        ++height;
                    }
                }

                for (int hv = 0; hv < height; ++hv) {
                    for (int w = 0; w < width; ++w) {
                        mask[u + w + (v + hv) * uLimit] = 0;
                    }
                }

                glm::vec3 base(0.0f);
                base[axis] = static_cast<float>(k) + (positive ? 1.0f : 0.0f);
                base[uAxis] = static_cast<float>(u);
                base[vAxis] = static_cast<float>(v);

                const glm::vec3 uDir = getUAxis(face);
                const glm::vec3 vDir = getVAxis(face);

                if (uDir[uAxis] < 0.0f) {
                    base[uAxis] += 1.0f;
                }
                if (vDir[vAxis] < 0.0f) {
                    base[vAxis] += 1.0f;
                }

                glm::vec3 uVec = uDir * static_cast<float>(width);
                glm::vec3 vVec = vDir * static_cast<float>(height);

                const auto uvs = getBlockUVs(atlas, blockId, face);
                const glm::vec3 normal = getNormal(face);

                const uint32_t baseIndex = static_cast<uint32_t>(vertices.size());

                vertices.push_back({base, normal, uvs[0]});
                vertices.push_back({base + uVec, normal, uvs[1]});
                vertices.push_back({base + vVec, normal, uvs[2]});
                vertices.push_back({base + uVec + vVec, normal, uvs[3]});

                if (positive) {
                    indices.push_back(baseIndex + 0);
                    indices.push_back(baseIndex + 1);
                    indices.push_back(baseIndex + 2);
                    indices.push_back(baseIndex + 2);
                    indices.push_back(baseIndex + 1);
                    indices.push_back(baseIndex + 3);
                } else {
                    indices.push_back(baseIndex + 0);
                    indices.push_back(baseIndex + 2);
                    indices.push_back(baseIndex + 1);
                    indices.push_back(baseIndex + 2);
                    indices.push_back(baseIndex + 3);
                    indices.push_back(baseIndex + 1);
                }

                u += width;
            }
        }
    }
}

bool ChunkMesh::shouldRenderFace(uint16_t blockId, uint16_t neighborId) const {
    if (blockId == 0) {
        return false;
    }

    const auto& registry = BlockRegistry::getInstance();
    const BlockType& block = registry.getBlock(blockId);

    if (!block.isOpaque && !block.isTransparent) {
        return false;
    }

    if (neighborId == blockId) {
        return false;
    }

    if (neighborId == 0) {
        return true;
    }

    const BlockType& neighbor = registry.getBlock(neighborId);
    if (neighbor.isOpaque) {
        return false;
    }

    if (block.isTransparent && neighbor.isTransparent) {
        return false;
    }

    return true;
}

std::array<glm::vec2, 4> ChunkMesh::getBlockUVs(TextureAtlas& atlas,
                                                 uint16_t blockId,
                                                 MeshFaceDirection face) const {
    const BlockType& block = BlockRegistry::getInstance().getBlock(blockId);
    const BlockFace blockFace = toBlockFace(face);
    const AtlasEntry* entry = atlas.getEntry(block.getTextureName(blockFace));

    if (!entry) {
        PC_WARN("Missing texture atlas entry for block '" + block.name +
                "' face: " + std::to_string(static_cast<int>(blockFace)));
        return {glm::vec2(0.0f, 0.0f),
                glm::vec2(1.0f, 0.0f),
                glm::vec2(0.0f, 1.0f),
                glm::vec2(1.0f, 1.0f)};
    }

    return {glm::vec2(entry->u0, entry->v0),
            glm::vec2(entry->u1, entry->v0),
            glm::vec2(entry->u0, entry->v1),
            glm::vec2(entry->u1, entry->v1)};
}

uint16_t ChunkMesh::getBlockFromNeighbors(const Chunk& chunk,
                                          ChunkManager& manager,
                                          int32_t x,
                                          int32_t y,
                                          int32_t z) const {
    if (x >= 0 && x < Chunk::CHUNK_SIZE_X && y >= 0 && y < Chunk::CHUNK_SIZE_Y &&
        z >= 0 && z < Chunk::CHUNK_SIZE_Z) {
        return chunk.getBlock(x, y, z);
    }

    if (y < 0 || y >= Chunk::CHUNK_SIZE_Y) {
        return 0;
    }

    ChunkCoord neighborCoord = chunk.getPosition();
    int localX = x;
    int localZ = z;

    if (localX < 0) {
        neighborCoord.x -= 1;
        localX += Chunk::CHUNK_SIZE_X;
    } else if (localX >= Chunk::CHUNK_SIZE_X) {
        neighborCoord.x += 1;
        localX -= Chunk::CHUNK_SIZE_X;
    }

    if (localZ < 0) {
        neighborCoord.z -= 1;
        localZ += Chunk::CHUNK_SIZE_Z;
    } else if (localZ >= Chunk::CHUNK_SIZE_Z) {
        neighborCoord.z += 1;
        localZ -= Chunk::CHUNK_SIZE_Z;
    }

    const Chunk* neighbor = manager.getChunk(neighborCoord);
    if (!neighbor) {
        return 0;
    }

    return neighbor->getBlock(localX, y, localZ);
}

glm::vec3 ChunkMesh::getNormal(MeshFaceDirection face) {
    switch (face) {
    case MeshFaceDirection::POS_X:
        return {1.0f, 0.0f, 0.0f};
    case MeshFaceDirection::NEG_X:
        return {-1.0f, 0.0f, 0.0f};
    case MeshFaceDirection::POS_Y:
        return {0.0f, 1.0f, 0.0f};
    case MeshFaceDirection::NEG_Y:
        return {0.0f, -1.0f, 0.0f};
    case MeshFaceDirection::POS_Z:
        return {0.0f, 0.0f, 1.0f};
    case MeshFaceDirection::NEG_Z:
        return {0.0f, 0.0f, -1.0f};
    }

    return {0.0f, 1.0f, 0.0f};
}

glm::vec3 ChunkMesh::getUAxis(MeshFaceDirection face) {
    switch (face) {
    case MeshFaceDirection::POS_X:
        return {0.0f, 0.0f, 1.0f};
    case MeshFaceDirection::NEG_X:
        return {0.0f, 0.0f, -1.0f};
    case MeshFaceDirection::POS_Y:
        return {1.0f, 0.0f, 0.0f};
    case MeshFaceDirection::NEG_Y:
        return {1.0f, 0.0f, 0.0f};
    case MeshFaceDirection::POS_Z:
        return {1.0f, 0.0f, 0.0f};
    case MeshFaceDirection::NEG_Z:
        return {-1.0f, 0.0f, 0.0f};
    }

    return {1.0f, 0.0f, 0.0f};
}

glm::vec3 ChunkMesh::getVAxis(MeshFaceDirection face) {
    switch (face) {
    case MeshFaceDirection::POS_X:
        return {0.0f, 1.0f, 0.0f};
    case MeshFaceDirection::NEG_X:
        return {0.0f, 1.0f, 0.0f};
    case MeshFaceDirection::POS_Y:
        return {0.0f, 0.0f, 1.0f};
    case MeshFaceDirection::NEG_Y:
        return {0.0f, 0.0f, -1.0f};
    case MeshFaceDirection::POS_Z:
        return {0.0f, 1.0f, 0.0f};
    case MeshFaceDirection::NEG_Z:
        return {0.0f, 1.0f, 0.0f};
    }

    return {0.0f, 1.0f, 0.0f};
}

BlockFace ChunkMesh::toBlockFace(MeshFaceDirection face) {
    switch (face) {
    case MeshFaceDirection::POS_X:
        return BlockFace::RIGHT;
    case MeshFaceDirection::NEG_X:
        return BlockFace::LEFT;
    case MeshFaceDirection::POS_Y:
        return BlockFace::TOP;
    case MeshFaceDirection::NEG_Y:
        return BlockFace::BOTTOM;
    case MeshFaceDirection::POS_Z:
        return BlockFace::BACK;
    case MeshFaceDirection::NEG_Z:
        return BlockFace::FRONT;
    }

    return BlockFace::FRONT;
}

int ChunkMesh::axisIndex(MeshFaceDirection face) {
    switch (face) {
    case MeshFaceDirection::POS_X:
    case MeshFaceDirection::NEG_X:
        return 0;
    case MeshFaceDirection::POS_Y:
    case MeshFaceDirection::NEG_Y:
        return 1;
    case MeshFaceDirection::POS_Z:
    case MeshFaceDirection::NEG_Z:
        return 2;
    }

    return 0;
}

int ChunkMesh::uAxisIndex(MeshFaceDirection face) {
    switch (face) {
    case MeshFaceDirection::POS_X:
    case MeshFaceDirection::NEG_X:
        return 2;
    case MeshFaceDirection::POS_Y:
    case MeshFaceDirection::NEG_Y:
        return 0;
    case MeshFaceDirection::POS_Z:
    case MeshFaceDirection::NEG_Z:
        return 0;
    }

    return 0;
}

int ChunkMesh::vAxisIndex(MeshFaceDirection face) {
    switch (face) {
    case MeshFaceDirection::POS_X:
    case MeshFaceDirection::NEG_X:
        return 1;
    case MeshFaceDirection::POS_Y:
    case MeshFaceDirection::NEG_Y:
        return 2;
    case MeshFaceDirection::POS_Z:
    case MeshFaceDirection::NEG_Z:
        return 1;
    }

    return 1;
}

bool ChunkMesh::isPositiveDirection(MeshFaceDirection face) {
    switch (face) {
    case MeshFaceDirection::POS_X:
    case MeshFaceDirection::POS_Y:
    case MeshFaceDirection::POS_Z:
        return true;
    case MeshFaceDirection::NEG_X:
    case MeshFaceDirection::NEG_Y:
    case MeshFaceDirection::NEG_Z:
        return false;
    }

    return true;
}

} // namespace PoorCraft
