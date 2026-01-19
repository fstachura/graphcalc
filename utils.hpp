#pragma once
#include <GL/gl.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>

#include <glm/glm.hpp>
#include "GLFW/glfw3.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
};

class GLCamera {
public:
    glm::vec3 position { 0, 0, 0 };
    glm::vec3 up { 0, 1, 0 };
    glm::vec3 where { 0, 0, 0 };
    float aspectRatio = 4.05 / 3.0f;
    // float aspectRatio = 1200.0f / 800.0f;
    float fieldOfView = 80.f;
    float zNear = 0.1f;
    float zFar = 500.f;

    GLCamera() { }

    glm::mat4 getViewMatrix() const {
        // TODO math behind this
        return glm::lookAt(position, where, up);
    }

    glm::mat4 getProjectionMatrix() const {
        return glm::perspective(glm::radians(fieldOfView), aspectRatio, zNear, zFar);
    }

    void setAspectRatio(float aspectRatio) {
        this->aspectRatio = aspectRatio;
    }
};

struct GLMesh {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
};

struct GLRenderable {
    virtual void render(const GLCamera& camera) = 0;
    virtual ~GLRenderable() {}
};

struct DirectionalLight {
    glm::vec3 direction = {-1.0f, 0.0, -1.0f};
    glm::vec3 color = {1.0f, 1.0, 1.0f};

    DirectionalLight(const DirectionalLight&) = default;
};

struct PointLight {
    glm::vec3 position = {0.0, 10.0f, 0.0};
    glm::vec3 color = {1.0f, 1.0, 1.0f};
    float att_constant = 1.0f;
    float att_linear = 1.0f;
    float att_quadratic = 1.0f;

    PointLight(const PointLight&) = default;
};

GLMesh generate_plane_mesh(int side_len) {
    GLMesh plane;

    double start_x = -1.0;
    double end_x = 1.0;
    double step_x = (end_x-start_x) / ((float)side_len-1);
    double start_y = -1.0;
    double end_y = 1.0;
    double step_y = (end_y-start_y) / ((float)side_len-1);
    double offset_x = 0, offset_y = 0;

    for (int y=0; y != side_len; y++) {
        offset_y = 0.0;
        for (int x=0; x != side_len; x++) {
            plane.vertices.push_back({
                { start_x + offset_x, 0.0, start_y + offset_y },
                {
                    (y*side_len + x) % 3 == 0 ? 0.0 : 1.0,
                    (y*(side_len+1) + x) % 3 == 1 ? 0.0 : 1.0,
                    (y*(side_len+2) + x) % 3 == 2 ? 0.0 : 1.0,
                }
            });
            offset_y += step_y;
        }
        offset_x += step_x;
    }

    for (int y=0; y != side_len-1; y++) {
        for (int x=0; x != side_len-1; x++) {
            plane.indices.push_back(y*side_len + x);
            plane.indices.push_back(y*side_len + x + 1);
            plane.indices.push_back((y+1)*side_len + x);

            plane.indices.push_back((y+1)*side_len + x) ;
            plane.indices.push_back((y+1)*side_len + x + 1);
            plane.indices.push_back(y*side_len + x + 1);
        }
    }

    return plane;
}

struct TextureLoadError: public std::exception {
    std::string path;

    TextureLoadError(std::string path): path(path) {
    }
};

unsigned int createGLTexture(const char* path) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data == nullptr) {
        throw TextureLoadError(path);
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);

    return texture;
}

unsigned int create1DGLTexture(const char* path) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, texture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data == nullptr) {
        throw TextureLoadError(path);
    }

    // 0 - level of detail (base detail level)
    // second 0 - border
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, width, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);

    return texture;
}

unsigned int createGLBuffer(unsigned int size) {
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, buffer);
    glNamedBufferData(buffer, size, NULL, GL_DYNAMIC_READ);
    return buffer;
}

std::string readFile(const std::filesystem::path &path) {
    std::ifstream stream(path, std::ios::in);
    if (!stream.is_open()) {
        throw std::runtime_error("failed to open shader " + path.string());
    }

    std::stringstream sstr;
    sstr << stream.rdbuf();
    std::string data = sstr.str();
    stream.close();

    return data;
}
