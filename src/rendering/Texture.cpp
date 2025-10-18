#include "poorcraft/rendering/Texture.h"

#include <algorithm>

#include <glad/glad.h>

#include "poorcraft/core/Logger.h"
#include "poorcraft/platform/Platform.h"

#include "stb_image.h"

namespace PoorCraft {

namespace {
TextureFormat channelsToFormat(int channels) {
    switch (channels) {
        case 1:
            return TextureFormat::RED;
        case 2:
            return TextureFormat::RG;
        case 3:
            return TextureFormat::RGB;
        case 4:
            return TextureFormat::RGBA;
        default:
            return TextureFormat::RGBA;
    }
}
}

Texture::Texture(const std::string& path, const TextureParams& params)
    : Resource(path), m_Params(params) {}

bool Texture::load() {
    if (m_TextureID != 0) {
        unload();
    }

    setState(ResourceState::Loading);

    if (!poorcraft::Platform::file_exists(m_Path)) {
        PC_ERRORF("Texture file '%s' not found", m_Path.c_str());
        setState(ResourceState::Failed);
        return false;
    }

    stbi_set_flip_vertically_on_load(false);

    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* data = stbi_load(m_Path.c_str(), &width, &height, &channels, 0);
    if (!data) {
        PC_ERRORF("Failed to load texture '%s': %s", m_Path.c_str(), stbi_failure_reason());
        setState(ResourceState::Failed);
        return false;
    }

    TextureFormat format = channelsToFormat(channels);
    GLenum glFormat = getGLFormat(format);
    GLenum glInternalFormat = getGLInternalFormat(format);

    GLuint textureID = 0;
    glGenTextures(1, &textureID);
    m_Target = GL_TEXTURE_2D;
    glBindTexture(m_Target, textureID);

    glTexImage2D(m_Target, 0, glInternalFormat, width, height, 0, glFormat, GL_UNSIGNED_BYTE, data);

    glTexParameteri(m_Target, GL_TEXTURE_MIN_FILTER, getGLFilter(m_Params.minFilter));
    glTexParameteri(m_Target, GL_TEXTURE_MAG_FILTER, getGLFilter(m_Params.magFilter));
    glTexParameteri(m_Target, GL_TEXTURE_WRAP_S, getGLWrap(m_Params.wrapS));
    glTexParameteri(m_Target, GL_TEXTURE_WRAP_T, getGLWrap(m_Params.wrapT));

    if (m_Params.generateMipmaps) {
        glGenerateMipmap(m_Target);
    }

    if (m_Params.anisotropicFiltering > 0.0f) {
        if (GLAD_GL_EXT_texture_filter_anisotropic) {
            GLfloat maxAniso = 0.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
            float aniso = std::min(m_Params.anisotropicFiltering, static_cast<float>(maxAniso));
            glTexParameterf(m_Target, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
        } else {
            PC_WARN("Anisotropic filtering requested but EXT_texture_filter_anisotropic not supported");
        }
    }

    stbi_image_free(data);

    m_TextureID = textureID;
    m_Width = width;
    m_Height = height;
    m_Channels = channels;
    setSize(static_cast<size_t>(width) * static_cast<size_t>(height) * static_cast<size_t>(channels));
    setState(ResourceState::Loaded);

    PC_INFOF("Texture '%s' loaded (%dx%d, channels: %d, ID: %u)", m_Path.c_str(), m_Width, m_Height, m_Channels, m_TextureID);

    return true;
}

void Texture::unload() {
    if (m_TextureID != 0) {
        glDeleteTextures(1, &m_TextureID);
        PC_INFOF("Texture '%s' unloaded (ID: %u)", m_Path.c_str(), m_TextureID);
        m_TextureID = 0;
    }

    m_Width = 0;
    m_Height = 0;
    m_Channels = 0;
    setSize(0);
    setState(ResourceState::Unloaded);
}

ResourceType Texture::getType() const {
    return ResourceType::Texture;
}

void Texture::bind(uint32_t slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(m_Target, m_TextureID);
}

void Texture::unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
}

int Texture::getWidth() const {
    return m_Width;
}

int Texture::getHeight() const {
    return m_Height;
}

int Texture::getChannels() const {
    return m_Channels;
}

GLuint Texture::getTextureID() const {
    return m_TextureID;
}

std::shared_ptr<Texture> Texture::createFromData(int width,
                                                int height,
                                                TextureFormat format,
                                                const void* data,
                                                const TextureParams& params) {
    auto texture = std::shared_ptr<Texture>(new Texture("", params));
    texture->m_Width = width;
    texture->m_Height = height;
    texture->m_Channels = texture->getChannelCount(format);

    glGenTextures(1, &texture->m_TextureID);
    texture->m_Target = GL_TEXTURE_2D;
    glBindTexture(texture->m_Target, texture->m_TextureID);

    GLenum internalFormat = texture->getGLInternalFormat(format);
    GLenum glFormat = texture->getGLFormat(format);
    GLenum dataType = texture->getGLDataType(format);

    glTexImage2D(texture->m_Target,
                 0,
                 internalFormat,
                 width,
                 height,
                 0,
                 glFormat,
                 dataType,
                 data);

    glTexParameteri(texture->m_Target, GL_TEXTURE_MIN_FILTER, texture->getGLFilter(params.minFilter));
    glTexParameteri(texture->m_Target, GL_TEXTURE_MAG_FILTER, texture->getGLFilter(params.magFilter));
    glTexParameteri(texture->m_Target, GL_TEXTURE_WRAP_S, texture->getGLWrap(params.wrapS));
    glTexParameteri(texture->m_Target, GL_TEXTURE_WRAP_T, texture->getGLWrap(params.wrapT));

    if (params.generateMipmaps) {
        glGenerateMipmap(texture->m_Target);
    }

    if (params.anisotropicFiltering > 0.0f) {
        if (GLAD_GL_EXT_texture_filter_anisotropic) {
            GLfloat maxAniso = 0.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
            float aniso = std::min(params.anisotropicFiltering, static_cast<float>(maxAniso));
            glTexParameterf(texture->m_Target, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
        } else {
            PC_WARN("Anisotropic filtering requested but EXT_texture_filter_anisotropic not supported");
        }
    }

    Texture::unbind();

    texture->setState(ResourceState::Loaded);
    size_t bytesPerChannel = 1;
    switch (dataType) {
        case GL_FLOAT:
            bytesPerChannel = sizeof(float);
            break;
        case GL_UNSIGNED_INT_24_8:
            bytesPerChannel = 4;
            break;
        default:
            bytesPerChannel = sizeof(uint8_t);
            break;
    }
    size_t channels = static_cast<size_t>(std::max(1, texture->m_Channels));
    texture->setSize(static_cast<size_t>(width) * static_cast<size_t>(height) * channels * bytesPerChannel);

    return texture;
}

std::shared_ptr<Texture> Texture::createMultisample(int width,
                                                    int height,
                                                    TextureFormat format,
                                                    int samples) {
    auto texture = std::shared_ptr<Texture>(new Texture("", {}));
    texture->m_Width = width;
    texture->m_Height = height;
    texture->m_Channels = texture->getChannelCount(format);
    texture->m_Target = GL_TEXTURE_2D_MULTISAMPLE;

    glGenTextures(1, &texture->m_TextureID);
    glBindTexture(texture->m_Target, texture->m_TextureID);

    GLenum internalFormat = texture->getGLInternalFormat(format);
    GLenum glFormat = texture->getGLFormat(format);

    glTexImage2DMultisample(texture->m_Target,
                            samples,
                            internalFormat,
                            width,
                            height,
                            GL_TRUE);

    Texture::unbind();

    texture->setState(ResourceState::Loaded);
    size_t channels = static_cast<size_t>(std::max(1, texture->m_Channels));
    texture->setSize(static_cast<size_t>(width) * static_cast<size_t>(height) * channels);

    return texture;
}

GLenum Texture::getGLFormat(TextureFormat format) const {
    switch (format) {
        case TextureFormat::RGB:
            return GL_RGB;
        case TextureFormat::RGBA:
            return GL_RGBA;
        case TextureFormat::RED:
            return GL_RED;
        case TextureFormat::RG:
            return GL_RG;
        case TextureFormat::DEPTH:
            return GL_DEPTH_COMPONENT;
        case TextureFormat::DEPTH_STENCIL:
            return GL_DEPTH_STENCIL;
        default:
            return GL_RGBA;
    }
}

GLenum Texture::getGLInternalFormat(TextureFormat format) const {
    switch (format) {
        case TextureFormat::RGB:
            return GL_RGB8;
        case TextureFormat::RGBA:
            return GL_RGBA8;
        case TextureFormat::RED:
            return GL_R8;
        case TextureFormat::RG:
            return GL_RG8;
        case TextureFormat::DEPTH:
            return GL_DEPTH_COMPONENT24;
        case TextureFormat::DEPTH_STENCIL:
            return GL_DEPTH24_STENCIL8;
        default:
            return GL_RGBA8;
    }
}

GLenum Texture::getGLFilter(TextureFilter filter) const {
    switch (filter) {
        case TextureFilter::NEAREST:
            return GL_NEAREST;
        case TextureFilter::LINEAR:
            return GL_LINEAR;
        case TextureFilter::NEAREST_MIPMAP_NEAREST:
            return GL_NEAREST_MIPMAP_NEAREST;
        case TextureFilter::LINEAR_MIPMAP_LINEAR:
            return GL_LINEAR_MIPMAP_LINEAR;
        default:
            return GL_LINEAR;
    }
}

GLenum Texture::getGLWrap(TextureWrap wrap) const {
    switch (wrap) {
        case TextureWrap::REPEAT:
            return GL_REPEAT;
        case TextureWrap::CLAMP_TO_EDGE:
            return GL_CLAMP_TO_EDGE;
        case TextureWrap::CLAMP_TO_BORDER:
            return GL_CLAMP_TO_BORDER;
        case TextureWrap::MIRRORED_REPEAT:
            return GL_MIRRORED_REPEAT;
        default:
            return GL_REPEAT;
    }
}

int Texture::getChannelCount(TextureFormat format) const {
    switch (format) {
        case TextureFormat::RGB:
            return 3;
        case TextureFormat::RGBA:
            return 4;
        case TextureFormat::RED:
            return 1;
        case TextureFormat::RG:
            return 2;
        case TextureFormat::DEPTH:
        case TextureFormat::DEPTH_STENCIL:
            return 1;
        default:
            return 4;
    }
}

GLenum Texture::getGLDataType(TextureFormat format) const {
    switch (format) {
        case TextureFormat::DEPTH:
            return GL_FLOAT;
        case TextureFormat::DEPTH_STENCIL:
            return GL_UNSIGNED_INT_24_8;
        default:
            return GL_UNSIGNED_BYTE;
    }
}

} // namespace PoorCraft
