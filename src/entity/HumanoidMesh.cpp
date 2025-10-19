#include "poorcraft/entity/HumanoidMesh.h"

#include <array>

#include "poorcraft/core/Logger.h"

namespace PoorCraft {

namespace {
struct Face {
    glm::vec3 normal;
    std::array<int, 4> indices;
    PlayerSkin::Face faceId;
};

const std::array<Face, 6> FACES = { {
    { glm::vec3(0.0f, 0.0f, 1.0f), { 0, 1, 2, 3 }, PlayerSkin::Face::Front },  // Front
    { glm::vec3(0.0f, 0.0f, -1.0f), { 4, 5, 6, 7 }, PlayerSkin::Face::Back },   // Back
    { glm::vec3(-1.0f, 0.0f, 0.0f), { 4, 0, 3, 7 }, PlayerSkin::Face::Left },   // Left
    { glm::vec3(1.0f, 0.0f, 0.0f), { 1, 5, 6, 2 }, PlayerSkin::Face::Right },   // Right
    { glm::vec3(0.0f, 1.0f, 0.0f), { 3, 2, 6, 7 }, PlayerSkin::Face::Top },     // Top
    { glm::vec3(0.0f, -1.0f, 0.0f), { 4, 5, 1, 0 }, PlayerSkin::Face::Bottom }  // Bottom
} };

std::array<glm::vec3, 8> computeCorners(const glm::vec3& min, const glm::vec3& max) {
    return {
        glm::vec3(min.x, min.y, max.z), // 0 Front-bottom-left
        glm::vec3(max.x, min.y, max.z), // 1 Front-bottom-right
        glm::vec3(max.x, max.y, max.z), // 2 Front-top-right
        glm::vec3(min.x, max.y, max.z), // 3 Front-top-left
        glm::vec3(min.x, min.y, min.z), // 4 Back-bottom-left
        glm::vec3(max.x, min.y, min.z), // 5 Back-bottom-right
        glm::vec3(max.x, max.y, min.z), // 6 Back-top-right
        glm::vec3(min.x, max.y, min.z)  // 7 Back-top-left
    };
}

std::shared_ptr<VertexArray> createVertexArray(const std::vector<HumanoidMesh::Vertex>& vertices,
                                               const std::vector<uint32_t>& indices) {
    auto vao = std::make_shared<VertexArray>();
    vao->create();

    std::vector<VertexAttribute> attributes = {
        { 0, 3, VertexAttributeType::FLOAT, false, sizeof(HumanoidMesh::Vertex), offsetof(HumanoidMesh::Vertex, position) },
        { 1, 3, VertexAttributeType::FLOAT, false, sizeof(HumanoidMesh::Vertex), offsetof(HumanoidMesh::Vertex, normal) },
        { 2, 2, VertexAttributeType::FLOAT, false, sizeof(HumanoidMesh::Vertex), offsetof(HumanoidMesh::Vertex, uv) }
    };

    vao->addVertexBuffer(vertices.data(), vertices.size() * sizeof(HumanoidMesh::Vertex), attributes);
    vao->setIndexBuffer(indices.data(), indices.size());

    return vao;
}

} // namespace

HumanoidMesh::MeshData HumanoidMesh::generate(const PlayerSkin* skin) {
    PlayerSkin::SkinLayout layout{};
    if (skin) {
        layout = skin->getSkinLayout();
    } else {
        auto makeUniformRegion = []() {
            PlayerSkin::Region region;
            PlayerSkin::FaceRegion uniform{ glm::vec2(0.0f), glm::vec2(1.0f) };
            region.front = uniform;
            region.back = uniform;
            region.left = uniform;
            region.right = uniform;
            region.top = uniform;
            region.bottom = uniform;
            return region;
        };

        layout.head = makeUniformRegion();
        layout.body = makeUniformRegion();
        layout.leftArm = makeUniformRegion();
        layout.rightArm = makeUniformRegion();
        layout.leftLeg = makeUniformRegion();
        layout.rightLeg = makeUniformRegion();
        PC_WARN("HumanoidMesh::generate using default layout (no skin provided)");
    }

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    vertices.reserve(24 * 6);
    indices.reserve(36 * 6);

    std::array<MeshSection, 6> sections{};
    sections[0] = addCube(vertices, indices, glm::vec3(-0.125f, 1.25f, -0.125f), glm::vec3(0.125f, 1.5f, 0.125f), layout.head);
    sections[1] = addCube(vertices, indices, glm::vec3(-0.125f, 0.75f, -0.0625f), glm::vec3(0.125f, 1.25f, 0.0625f), layout.body);
    sections[2] = addCube(vertices, indices, glm::vec3(-0.25f, 0.75f, -0.0625f), glm::vec3(-0.125f, 1.25f, 0.0625f), layout.leftArm);
    sections[3] = addCube(vertices, indices, glm::vec3(0.125f, 0.75f, -0.0625f), glm::vec3(0.25f, 1.25f, 0.0625f), layout.rightArm);
    sections[4] = addCube(vertices, indices, glm::vec3(-0.125f, 0.0f, -0.0625f), glm::vec3(-0.0625f, 0.75f, 0.0625f), layout.leftLeg);
    sections[5] = addCube(vertices, indices, glm::vec3(0.0625f, 0.0f, -0.0625f), glm::vec3(0.125f, 0.75f, 0.0625f), layout.rightLeg);

    PC_INFOF("Humanoid mesh generated (vertices: %zu, indices: %zu)", vertices.size(), indices.size());

    MeshData data;
    data.mesh = createVertexArray(vertices, indices);
    data.sections = sections;
    return data;
}

HumanoidMesh::MeshSection HumanoidMesh::addCube(std::vector<Vertex>& vertices,
                                                std::vector<uint32_t>& indices,
                                                const glm::vec3& min,
                                                const glm::vec3& max,
                                                const PlayerSkin::Region& region) {
    const auto corners = computeCorners(min, max);

    MeshSection section;
    section.indexOffset = static_cast<uint32_t>(indices.size());

    for (const auto& face : FACES) {
        const auto& faceRegion = region.getFace(face.faceId);
        const glm::vec2 uvMin = faceRegion.uvMin;
        const glm::vec2 uvMax = faceRegion.uvMax;

        const uint32_t baseIndex = static_cast<uint32_t>(vertices.size());

        std::array<glm::vec2, 4> faceUVs = {
            glm::vec2(uvMin.x, uvMin.y),
            glm::vec2(uvMax.x, uvMin.y),
            glm::vec2(uvMax.x, uvMax.y),
            glm::vec2(uvMin.x, uvMax.y)
        };

        for (std::size_t i = 0; i < face.indices.size(); ++i) {
            Vertex vertex;
            vertex.position = corners[face.indices[i]];
            vertex.normal = face.normal;
            vertex.uv = faceUVs[i];
            vertices.push_back(vertex);
        }

        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 0);
    }

    section.indexCount = static_cast<uint32_t>(indices.size()) - section.indexOffset;
    return section;
}

} // namespace PoorCraft
