#pragma once

#include <cstdint>
#include <vector>

#include <glad/glad.h>

namespace PoorCraft {

enum class VertexAttributeType {
    FLOAT,
    INT,
    UINT,
    BYTE,
    UBYTE
};

struct VertexAttribute {
    uint32_t index = 0;
    int size = 0;
    VertexAttributeType type = VertexAttributeType::FLOAT;
    bool normalized = false;
    std::size_t stride = 0;
    std::size_t offset = 0;
};

enum class BufferUsage {
    STATIC_DRAW,
    DYNAMIC_DRAW,
    STREAM_DRAW
};

class VertexArray {
public:
    VertexArray();
    ~VertexArray();

    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;
    VertexArray(VertexArray&&) = default;
    VertexArray& operator=(VertexArray&&) = default;

    bool create();
    void destroy();

    void bind() const;
    static void unbind();

    std::size_t addVertexBuffer(const void* data,
                                std::size_t size,
                                const std::vector<VertexAttribute>& attributes,
                                BufferUsage usage = BufferUsage::STATIC_DRAW);

    bool setIndexBuffer(const uint32_t* indices,
                        std::size_t count,
                        BufferUsage usage = BufferUsage::STATIC_DRAW);

    bool updateVertexBuffer(std::size_t bufferIndex,
                            std::size_t offset,
                            const void* data,
                            std::size_t size);

    void draw(GLenum mode, std::size_t count, std::size_t offset = 0) const;
    void drawInstanced(GLenum mode, std::size_t count, std::size_t instanceCount) const;

    GLuint getVAO() const;
    std::size_t getIndexCount() const;
    bool hasIndices() const;

private:
    GLenum getGLType(VertexAttributeType type) const;
    GLenum getGLUsage(BufferUsage usage) const;

private:
    GLuint m_VAO = 0;
    std::vector<GLuint> m_VBOs;
    GLuint m_EBO = 0;
    std::size_t m_IndexCount = 0;
};

} // namespace PoorCraft
