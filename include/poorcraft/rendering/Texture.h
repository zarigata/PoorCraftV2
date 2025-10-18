#pragma once

#include <memory>
#include <string>

#include <glad/glad.h>

#include "poorcraft/resource/Resource.h"

namespace PoorCraft {

enum class TextureFormat {
    RGB,
    RGBA,
    RED,
    RG,
    DEPTH,
    DEPTH_STENCIL
};

enum class TextureFilter {
    NEAREST,
    LINEAR,
    NEAREST_MIPMAP_NEAREST,
    LINEAR_MIPMAP_LINEAR
};

enum class TextureWrap {
    REPEAT,
    CLAMP_TO_EDGE,
    CLAMP_TO_BORDER,
    MIRRORED_REPEAT
};

struct TextureParams {
    TextureFilter minFilter = TextureFilter::LINEAR;
    TextureFilter magFilter = TextureFilter::LINEAR;
    TextureWrap wrapS = TextureWrap::REPEAT;
    TextureWrap wrapT = TextureWrap::REPEAT;
    bool generateMipmaps = true;
    float anisotropicFiltering = 0.0f;
};

// Textures retain their original orientation (no vertical flip). Texture atlas loading
// follows the same policy to keep UV coordinates consistent across systems.
class Texture : public Resource {
public:
    Texture(const std::string& path, const TextureParams& params = {});
    ~Texture() override = default;

    bool load() override;
    void unload() override;
    ResourceType getType() const override;

    void bind(uint32_t slot = 0) const;
    static void unbind();

    int getWidth() const;
    int getHeight() const;
    int getChannels() const;
    GLuint getTextureID() const;
    GLenum getTarget() const { return m_Target; }

    static std::shared_ptr<Texture> createFromData(int width,
                                                   int height,
                                                   TextureFormat format,
                                                   const void* data,
                                                   const TextureParams& params = {});
    static std::shared_ptr<Texture> createMultisample(int width,
                                                      int height,
                                                      TextureFormat format,
                                                      int samples);

private:
    GLenum getGLFormat(TextureFormat format) const;
    GLenum getGLInternalFormat(TextureFormat format) const;
    GLenum getGLFilter(TextureFilter filter) const;
    GLenum getGLWrap(TextureWrap wrap) const;
    int getChannelCount(TextureFormat format) const;
    GLenum getGLDataType(TextureFormat format) const;

private:
    GLuint m_TextureID = 0;
    int m_Width = 0;
    int m_Height = 0;
    int m_Channels = 0;
    TextureParams m_Params;
    GLenum m_Target = GL_TEXTURE_2D;
};

} // namespace PoorCraft
