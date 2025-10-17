#include "poorcraft/rendering/VertexArray.h"

#include <algorithm>

#include "poorcraft/core/Logger.h"

namespace PoorCraft {

namespace {
bool isIntegerAttribute(VertexAttributeType type) {
    return type == VertexAttributeType::INT ||
           type == VertexAttributeType::UINT ||
           type == VertexAttributeType::BYTE ||
           type == VertexAttributeType::UBYTE;
}
}

VertexArray::VertexArray() {
    create();
}

VertexArray::~VertexArray() {
    destroy();
}

bool VertexArray::create() {
    if (m_VAO != 0) {
        return true;
    }

    glGenVertexArrays(1, &m_VAO);
    if (m_VAO == 0) {
        PC_ERROR("Failed to create VertexArray (VAO)" );
        return false;
    }

    PC_INFOF("VertexArray created (VAO: %u)", m_VAO);
    return true;
}

void VertexArray::destroy() {
    if (!m_VBOs.empty()) {
        glDeleteBuffers(static_cast<GLsizei>(m_VBOs.size()), m_VBOs.data());
        m_VBOs.clear();
    }

    if (m_EBO != 0) {
        glDeleteBuffers(1, &m_EBO);
        m_EBO = 0;
        m_IndexCount = 0;
    }

    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        PC_INFOF("VertexArray destroyed (VAO: %u)", m_VAO);
        m_VAO = 0;
    }
}

void VertexArray::bind() const {
    glBindVertexArray(m_VAO);
}

void VertexArray::unbind() {
    glBindVertexArray(0);
}

std::size_t VertexArray::addVertexBuffer(const void* data,
                                         std::size_t size,
                                         const std::vector<VertexAttribute>& attributes,
                                         BufferUsage usage) {
    if (m_VAO == 0) {
        if (!create()) {
            return std::numeric_limits<std::size_t>::max();
        }
    }

    bind();

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), data, getGLUsage(usage));

    for (const auto& attribute : attributes) {
        if (isIntegerAttribute(attribute.type)) {
            glVertexAttribIPointer(attribute.index,
                                   attribute.size,
                                   getGLType(attribute.type),
                                   static_cast<GLsizei>(attribute.stride),
                                   reinterpret_cast<const void*>(attribute.offset));
        } else {
            glVertexAttribPointer(attribute.index,
                                  attribute.size,
                                  getGLType(attribute.type),
                                  attribute.normalized ? GL_TRUE : GL_FALSE,
                                  static_cast<GLsizei>(attribute.stride),
                                  reinterpret_cast<const void*>(attribute.offset));
        }
        glEnableVertexAttribArray(attribute.index);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    unbind();

    m_VBOs.push_back(vbo);
    std::size_t bufferIndex = m_VBOs.size() - 1;

    PC_DEBUGF("Vertex buffer added to VAO %u (buffer index: %zu, size: %zu bytes, attributes: %zu)",
              m_VAO,
              bufferIndex,
              size,
              attributes.size());

    return bufferIndex;
}

bool VertexArray::setIndexBuffer(const uint32_t* indices,
                                 std::size_t count,
                                 BufferUsage usage) {
    if (m_VAO == 0) {
        if (!create()) {
            return false;
        }
    }

    bind();

    if (m_EBO == 0) {
        glGenBuffers(1, &m_EBO);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(count * sizeof(uint32_t)),
                 indices,
                 getGLUsage(usage));

    m_IndexCount = count;

    PC_DEBUGF("Index buffer set on VAO %u (indices: %zu)", m_VAO, m_IndexCount);

    unbind();
    return true;
}

bool VertexArray::updateVertexBuffer(std::size_t bufferIndex,
                                     std::size_t offset,
                                     const void* data,
                                     std::size_t size) {
    if (bufferIndex >= m_VBOs.size()) {
        PC_ERRORF("Vertex buffer index %zu out of range for VAO %u", bufferIndex, m_VAO);
        return false;
    }

    bind();
    GLuint vbo = m_VBOs[bufferIndex];
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    unbind();

    PC_TRACEF("Vertex buffer %zu updated (%zu bytes at offset %zu)", bufferIndex, size, offset);
    return true;
}

void VertexArray::draw(GLenum mode, std::size_t count, std::size_t offset) const {
    bind();
    if (hasIndices()) {
        std::size_t indexCount = count == 0 ? m_IndexCount : count;
        glDrawElements(mode,
                       static_cast<GLsizei>(indexCount),
                       GL_UNSIGNED_INT,
                       reinterpret_cast<const void*>(offset * sizeof(uint32_t)));
    } else {
        std::size_t vertexCount = count;
        glDrawArrays(mode, static_cast<GLint>(offset), static_cast<GLsizei>(vertexCount));
    }
    unbind();
}

void VertexArray::drawInstanced(GLenum mode, std::size_t count, std::size_t instanceCount) const {
    bind();
    if (hasIndices()) {
        glDrawElementsInstanced(mode,
                                static_cast<GLsizei>(count == 0 ? m_IndexCount : count),
                                GL_UNSIGNED_INT,
                                nullptr,
                                static_cast<GLsizei>(instanceCount));
    } else {
        glDrawArraysInstanced(mode,
                              0,
                              static_cast<GLsizei>(count),
                              static_cast<GLsizei>(instanceCount));
    }
    unbind();
}

GLuint VertexArray::getVAO() const {
    return m_VAO;
}

std::size_t VertexArray::getIndexCount() const {
    return m_IndexCount;
}

bool VertexArray::hasIndices() const {
    return m_EBO != 0 && m_IndexCount > 0;
}

GLenum VertexArray::getGLType(VertexAttributeType type) const {
    switch (type) {
        case VertexAttributeType::FLOAT:
            return GL_FLOAT;
        case VertexAttributeType::INT:
            return GL_INT;
        case VertexAttributeType::UINT:
            return GL_UNSIGNED_INT;
        case VertexAttributeType::BYTE:
            return GL_BYTE;
        case VertexAttributeType::UBYTE:
            return GL_UNSIGNED_BYTE;
        default:
            return GL_FLOAT;
    }
}

GLenum VertexArray::getGLUsage(BufferUsage usage) const {
    switch (usage) {
        case BufferUsage::STATIC_DRAW:
            return GL_STATIC_DRAW;
        case BufferUsage::DYNAMIC_DRAW:
            return GL_DYNAMIC_DRAW;
        case BufferUsage::STREAM_DRAW:
            return GL_STREAM_DRAW;
        default:
            return GL_STATIC_DRAW;
    }
}

} // namespace PoorCraft
