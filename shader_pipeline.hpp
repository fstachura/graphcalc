#pragma once
#include <optional>
#include <stdexcept>
#include <vector>
#include <map>

#include <glm/glm.hpp>
#include "GL/glew.h"

void checkShader(GLint id) {
    GLint result = GL_FALSE;
    int infoLogLength;

    // returns value of specific parameter object
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::vector<char> shaderErrorMessage(infoLogLength + 1);
        glGetShaderInfoLog(id, infoLogLength, nullptr, &shaderErrorMessage[0]);
        const std::string errorMessage = "shader compilation failed: " + std::string(&shaderErrorMessage[0]);
        throw std::runtime_error(errorMessage);
    }
}

void checkProgram(GLint id) {
    GLint result = GL_FALSE;
    int infoLogLength;

    // returns value of specific parameter object
    glGetProgramiv(id, GL_LINK_STATUS, &result);
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::vector<char> programErrorMessage(infoLogLength + 1);
        glGetProgramInfoLog(id, infoLogLength, nullptr, programErrorMessage.data());
        const std::string errorMessage = "shader linking failed: " + std::string(programErrorMessage.data());
        throw std::runtime_error(errorMessage);
    }
}


class GLShaderPipeline {
    GLuint id;
    bool linked = false;
    std::map<std::string, GLint> uniformIds {};
    std::optional<GLuint> vertexShaderId;
    std::optional<GLuint> fragmentShaderId;
    // tesselation shaders...
    // compute shaders?

    GLuint compileShader(const GLuint shaderKind, const std::string &shader) const {
        GLuint shaderID = glCreateShader(shaderKind);

        char const *vertexSourcePointer = shader.c_str();
        // shader id, how many code strings, actual code, array of string lengths
        glShaderSource(shaderID, 1, &vertexSourcePointer, nullptr);
        glCompileShader(shaderID);

        checkShader(shaderID);
        return shaderID;
    }

    GLint getUniformID(const std::string &name) {
        const auto it = uniformIds.find(name);
        if (it != uniformIds.end())
            return it->second;

        const GLint uniformId = glGetUniformLocation(id, name.c_str());
        if (uniformId == -1)
            throw std::runtime_error("failed to get uniform with name: " + name);

        uniformIds.emplace(name, uniformId);
        return uniformId;
    }

public:
    GLShaderPipeline() {
        id = glCreateProgram();
    }

    GLShaderPipeline(GLShaderPipeline&&) = delete;
    GLShaderPipeline(GLShaderPipeline&) = delete;

    void setVertexShader(const std::string &vertexShader) {
        auto oldShaderId = vertexShaderId;
        vertexShaderId = compileShader(GL_VERTEX_SHADER, vertexShader);
        if (oldShaderId.has_value()) {
            glDetachShader(id, *oldShaderId);
            glDeleteShader(*oldShaderId);
        }
        linked = false;
    }

    void setFragmentShader(const std::string &fragmentShader) {
        auto oldShaderId = fragmentShaderId;
        fragmentShaderId = compileShader(GL_FRAGMENT_SHADER, fragmentShader);
        if (oldShaderId.has_value()) {
            glDetachShader(id, *oldShaderId);
            glDeleteShader(*oldShaderId);
        }
        linked = false;
    }

    void enable() {
        if (!linked) {
            linkProgram();
            linked = true;
        }
        glUseProgram(id);
    }

    void setUniform(const std::string &name, const GLint value) {
        glUniform1i(getUniformID(name), value);
    }

    void setUniform(const std::string &name, const float value) {
        glUniform1f(getUniformID(name), value);
    }

    void setUniform(const std::string &name, const glm::vec2 &value) {
        glUniform2f(getUniformID(name), value.x, value.y);
    }

    void setUniform(const std::string &name, const glm::vec3 &value) {
        glUniform3f(getUniformID(name), value.x, value.y, value.z);
    }

    void setUniform(const std::string &name, const glm::vec4 &value) {
        glUniform4f(getUniformID(name), value.x, value.y, value.z, value.w);
    }

    void setUniform(const std::string &name, const glm::mat4 &value) {
        glUniformMatrix4fv(getUniformID(name), 1, GL_FALSE, &value[0][0]);
    }

    void setUniform(const std::string &name, const std::vector<GLint> &value) {
        glUniform1iv(getUniformID(name), static_cast<GLint>(value.size()), value.data());
    }

    void setUniform(const std::string &name, const std::vector<float> &value) {
        glUniform1fv(getUniformID(name), static_cast<GLint>(value.size()), value.data());
    }

    void linkProgram() const {
        if (vertexShaderId.has_value())
            glAttachShader(id, *vertexShaderId);
        if (fragmentShaderId.has_value())
            glAttachShader(id, *fragmentShaderId);
        glLinkProgram(id);
        checkProgram(id);
    }

    ~GLShaderPipeline() {
        glDeleteProgram(id);
        if (fragmentShaderId.has_value())
            glDeleteShader(*fragmentShaderId);
        if (vertexShaderId.has_value())
            glDeleteShader(*vertexShaderId);
    }
};

