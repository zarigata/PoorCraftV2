#include "poorcraft/rendering/Shader.h"

#include <vector>

#include "poorcraft/resource/Resource.h"

namespace PoorCraft {

namespace {
std::string shaderTypeToString(GLenum type) {
    switch (type) {
        case GL_VERTEX_SHADER:
            return "Vertex";
        case GL_FRAGMENT_SHADER:
            return "Fragment";
        case GL_GEOMETRY_SHADER:
            return "Geometry";
        case GL_COMPUTE_SHADER:
            return "Compute";
        case GL_TESS_CONTROL_SHADER:
            return "TessellationControl";
        case GL_TESS_EVALUATION_SHADER:
            return "TessellationEvaluation";
        default:
            return "Unknown";
    }
}
} // namespace

Shader::Shader(const std::string& basePath)
    : Resource(basePath) {
    m_VertexPath = m_Path + ".vert";
    m_FragmentPath = m_Path + ".frag";
}

bool Shader::load() {
    if (m_ProgramID != 0) {
        unload();
    }

    setState(ResourceState::Loading);

    std::string vertexSource;
    auto vertexResult = poorcraft::Platform::read_file_text(m_VertexPath, vertexSource);
    if (vertexResult != poorcraft::Platform::FileOperationResult::Success) {
        PC_ERRORF("Failed to read vertex shader '%s': %s", m_VertexPath.c_str(),
                  poorcraft::Platform::file_operation_result_to_string(vertexResult).c_str());
        setState(ResourceState::Failed);
        return false;
    }

    std::string fragmentSource;
    auto fragmentResult = poorcraft::Platform::read_file_text(m_FragmentPath, fragmentSource);
    if (fragmentResult != poorcraft::Platform::FileOperationResult::Success) {
        PC_ERRORF("Failed to read fragment shader '%s': %s", m_FragmentPath.c_str(),
                  poorcraft::Platform::file_operation_result_to_string(fragmentResult).c_str());
        setState(ResourceState::Failed);
        return false;
    }

    GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    if (vertexShader == 0) {
        setState(ResourceState::Failed);
        return false;
    }

    GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        setState(ResourceState::Failed);
        return false;
    }

    GLuint program = glCreateProgram();
    if (program == 0) {
        PC_ERRORF("Failed to create shader program for '%s'", m_Path.c_str());
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        setState(ResourceState::Failed);
        return false;
    }

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    if (!checkLinkErrors(program)) {
        glDetachShader(program, vertexShader);
        glDetachShader(program, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(program);
        setState(ResourceState::Failed);
        return false;
    }

    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    m_ProgramID = program;
    m_UniformLocationCache.clear();
    setState(ResourceState::Loaded);
    setSize(vertexSource.size() + fragmentSource.size());

    PC_INFOF("Shader '%s' loaded (Program ID: %u)", m_Path.c_str(), m_ProgramID);
    return true;
}

void Shader::unload() {
    if (m_ProgramID != 0) {
        glDeleteProgram(m_ProgramID);
        PC_INFOF("Shader '%s' unloaded (Program ID: %u)", m_Path.c_str(), m_ProgramID);
        m_ProgramID = 0;
    }
    m_UniformLocationCache.clear();
    setState(ResourceState::Unloaded);
    setSize(0);
}

ResourceType Shader::getType() const {
    return ResourceType::Shader;
}

void Shader::bind() const {
    if (m_ProgramID != 0) {
        glUseProgram(m_ProgramID);
    }
}

void Shader::unbind() {
    glUseProgram(0);
}

void Shader::setInt(const std::string& name, int value) {
    GLint location = obtainUniformLocation(name);
    if (location != -1) {
        glUniform1i(location, value);
    }
}

void Shader::setFloat(const std::string& name, float value) {
    GLint location = obtainUniformLocation(name);
    if (location != -1) {
        glUniform1f(location, value);
    }
}

void Shader::setBool(const std::string& name, bool value) {
    GLint location = obtainUniformLocation(name);
    if (location != -1) {
        glUniform1i(location, value ? 1 : 0);
    }
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) {
    GLint location = obtainUniformLocation(name);
    if (location != -1) {
        glUniform2fv(location, 1, &value[0]);
    }
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) {
    GLint location = obtainUniformLocation(name);
    if (location != -1) {
        glUniform3fv(location, 1, &value[0]);
    }
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) {
    GLint location = obtainUniformLocation(name);
    if (location != -1) {
        glUniform4fv(location, 1, &value[0]);
    }
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) {
    GLint location = obtainUniformLocation(name);
    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
    }
}

bool Shader::hasUniform(const std::string& name) {
    if (m_ProgramID == 0) {
        return false;
    }
    GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
    return location != -1;
}

GLuint Shader::getProgramID() const {
    return m_ProgramID;
}

bool Shader::reload() {
    PC_INFOF("Reloading shader '%s'", m_Path.c_str());
    unload();
    return load();
}

GLuint Shader::compileShader(const std::string& source, GLenum type) {
    GLuint shader = glCreateShader(type);
    if (shader == 0) {
        PC_ERRORF("Failed to create %s shader for '%s'", shaderTypeToString(type).c_str(), m_Path.c_str());
        return 0;
    }

    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    if (!checkCompileErrors(shader, type)) {
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

bool Shader::checkCompileErrors(GLuint shader, GLenum type) {
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> infoLog(static_cast<size_t>(logLength));
        glGetShaderInfoLog(shader, logLength, nullptr, infoLog.data());
        PC_ERRORF("%s shader compilation failed for '%s': %s",
                  shaderTypeToString(type).c_str(), m_Path.c_str(), infoLog.data());
        return false;
    }
    return true;
}

bool Shader::checkLinkErrors(GLuint program) {
    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        GLint logLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> infoLog(static_cast<size_t>(logLength));
        glGetProgramInfoLog(program, logLength, nullptr, infoLog.data());
        PC_ERRORF("Shader program link failed for '%s': %s", m_Path.c_str(), infoLog.data());
        return false;
    }
    return true;
}

GLint Shader::obtainUniformLocation(const std::string& name) {
    if (m_ProgramID == 0) {
        PC_WARNF("Shader '%s' has no program bound when setting uniform '%s'", m_Path.c_str(), name.c_str());
        return -1;
    }

    auto it = m_UniformLocationCache.find(name);
    if (it != m_UniformLocationCache.end()) {
        return it->second;
    }

    GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
    if (location == -1) {
        PC_WARNF("Uniform '%s' not found in shader '%s'", name.c_str(), m_Path.c_str());
        return -1;
    }

    m_UniformLocationCache.emplace(name, location);
    return location;
}

} // namespace PoorCraft
