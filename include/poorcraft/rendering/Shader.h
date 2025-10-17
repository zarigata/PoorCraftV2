#pragma once

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glad/glad.h>

#include "poorcraft/resource/Resource.h"
#include "poorcraft/platform/Platform.h"
#include "poorcraft/core/Logger.h"

namespace PoorCraft {

class Shader : public Resource {
public:
    explicit Shader(const std::string& basePath);
    ~Shader() override = default;

    bool load() override;
    void unload() override;
    ResourceType getType() const override;

    void bind() const;
    static void unbind();

    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);
    void setBool(const std::string& name, bool value);
    void setVec2(const std::string& name, const glm::vec2& value);
    void setVec3(const std::string& name, const glm::vec3& value);
    void setVec4(const std::string& name, const glm::vec4& value);
    void setMat4(const std::string& name, const glm::mat4& value);

    bool hasUniform(const std::string& name);
    GLuint getProgramID() const;

    bool reload();

private:
    GLuint compileShader(const std::string& source, GLenum type);
    bool checkCompileErrors(GLuint shader, GLenum type);
    bool checkLinkErrors(GLuint program);
    GLint obtainUniformLocation(const std::string& name);

private:
    GLuint m_ProgramID = 0;
    std::string m_VertexPath;
    std::string m_FragmentPath;
    std::unordered_map<std::string, GLint> m_UniformLocationCache;
};

} // namespace PoorCraft
