#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <glad/glad.h>

#include "poorcraft/rendering/Texture.h"

namespace PoorCraft {

enum class AttachmentType {
    COLOR,
    DEPTH,
    DEPTH_STENCIL
};

struct FramebufferSpec {
    uint32_t width = 0;
    uint32_t height = 0;
    int samples = 1;
    std::vector<AttachmentType> attachments;
    bool swapChainTarget = false;
};

class Framebuffer {
public:
    explicit Framebuffer(const FramebufferSpec& spec);
    ~Framebuffer();

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;
    Framebuffer(Framebuffer&&) = default;
    Framebuffer& operator=(Framebuffer&&) = default;

    bool create();
    void destroy();

    void bind() const;
    static void unbind();

    bool resize(uint32_t width, uint32_t height);

    std::shared_ptr<Texture> getColorAttachment(size_t index) const;
    std::shared_ptr<Texture> getDepthAttachment() const;

    GLuint getFramebufferID() const;
    uint32_t getWidth() const;
    uint32_t getHeight() const;
    int getSamples() const;

    const FramebufferSpec& getSpecification() const;

private:
    std::shared_ptr<Texture> createAttachment(AttachmentType type) const;

private:
    FramebufferSpec m_Spec;
    GLuint m_FramebufferID = 0;
    std::vector<std::shared_ptr<Texture>> m_ColorAttachments;
    std::shared_ptr<Texture> m_DepthAttachment;
};

} // namespace PoorCraft
