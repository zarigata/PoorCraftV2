#include "poorcraft/rendering/Framebuffer.h"

#include <array>

#include "poorcraft/core/Logger.h"

namespace PoorCraft {

Framebuffer::Framebuffer(const FramebufferSpec& spec)
    : m_Spec(spec) {}

Framebuffer::~Framebuffer() {
    destroy();
}

bool Framebuffer::create() {
    destroy();

    if (m_Spec.width == 0 || m_Spec.height == 0) {
        PC_ERROR("Framebuffer dimensions must be greater than zero");
        return false;
    }

    glGenFramebuffers(1, &m_FramebufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);

    m_ColorAttachments.clear();
    m_DepthAttachment.reset();

    uint32_t colorIndex = 0;
    for (AttachmentType type : m_Spec.attachments) {
        if (type == AttachmentType::COLOR) {
            auto texture = createAttachment(type);
            if (!texture) {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                destroy();
                return false;
            }
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                   static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + colorIndex),
                                   GL_TEXTURE_2D,
                                   texture->getTextureID(),
                                   0);
            m_ColorAttachments.push_back(texture);
            ++colorIndex;
        } else if (type == AttachmentType::DEPTH) {
            auto texture = createAttachment(type);
            if (!texture) {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                destroy();
                return false;
            }
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                   GL_DEPTH_ATTACHMENT,
                                   GL_TEXTURE_2D,
                                   texture->getTextureID(),
                                   0);
            m_DepthAttachment = texture;
        } else if (type == AttachmentType::DEPTH_STENCIL) {
            auto texture = createAttachment(type);
            if (!texture) {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                destroy();
                return false;
            }
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                   GL_DEPTH_STENCIL_ATTACHMENT,
                                   GL_TEXTURE_2D,
                                   texture->getTextureID(),
                                   0);
            m_DepthAttachment = texture;
        }
    }

    if (!m_ColorAttachments.empty()) {
        std::vector<GLenum> drawBuffers;
        drawBuffers.reserve(m_ColorAttachments.size());
        for (size_t i = 0; i < m_ColorAttachments.size(); ++i) {
            drawBuffers.push_back(static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i));
        }
        glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
    } else {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        PC_ERRORF("Framebuffer incomplete: 0x%X", status);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        destroy();
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    PC_INFOF("Framebuffer created (ID: %u, %ux%u, color attachments: %zu)",
             m_FramebufferID,
             m_Spec.width,
             m_Spec.height,
             m_ColorAttachments.size());

    return true;
}

void Framebuffer::destroy() {
    if (m_FramebufferID != 0) {
        glDeleteFramebuffers(1, &m_FramebufferID);
        m_FramebufferID = 0;
    }
    m_ColorAttachments.clear();
    m_DepthAttachment.reset();
}

void Framebuffer::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);
    glViewport(0, 0, static_cast<GLsizei>(m_Spec.width), static_cast<GLsizei>(m_Spec.height));
}

void Framebuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool Framebuffer::resize(uint32_t width, uint32_t height) {
    if (m_Spec.width == width && m_Spec.height == height) {
        return true;
    }

    m_Spec.width = width;
    m_Spec.height = height;
    return create();
}

std::shared_ptr<Texture> Framebuffer::getColorAttachment(size_t index) const {
    if (index >= m_ColorAttachments.size()) {
        return nullptr;
    }
    return m_ColorAttachments[index];
}

std::shared_ptr<Texture> Framebuffer::getDepthAttachment() const {
    return m_DepthAttachment;
}

GLuint Framebuffer::getFramebufferID() const {
    return m_FramebufferID;
}

uint32_t Framebuffer::getWidth() const {
    return m_Spec.width;
}

uint32_t Framebuffer::getHeight() const {
    return m_Spec.height;
}

int Framebuffer::getSamples() const {
    return m_Spec.samples;
}

const FramebufferSpec& Framebuffer::getSpecification() const {
    return m_Spec;
}

std::shared_ptr<Texture> Framebuffer::createAttachment(AttachmentType type) const {
    TextureFormat format = TextureFormat::RGBA;
    if (type == AttachmentType::DEPTH) {
        format = TextureFormat::DEPTH;
    } else if (type == AttachmentType::DEPTH_STENCIL) {
        format = TextureFormat::DEPTH_STENCIL;
    }

    TextureParams params;
    params.generateMipmaps = false;
    params.anisotropicFiltering = 0.0f;
    params.minFilter = TextureFilter::LINEAR;
    params.magFilter = TextureFilter::LINEAR;
    params.wrapS = TextureWrap::CLAMP_TO_EDGE;
    params.wrapT = TextureWrap::CLAMP_TO_EDGE;

    auto texture = Texture::createFromData(static_cast<int>(m_Spec.width),
                                           static_cast<int>(m_Spec.height),
                                           format,
                                           nullptr,
                                           params);

    if (!texture) {
        PC_ERROR("Failed to create framebuffer attachment texture");
    }

    return texture;
}

} // namespace PoorCraft
