#pragma once
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>

#include <glm/glm.hpp>
#include "GLFW/glfw3.h"

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
};

struct GLMesh {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
};

struct GLRenderable {
    virtual void render(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix) = 0;
    virtual ~GLRenderable() {}
};

GLMesh generate_plane_mesh(int side_len) {
    GLMesh plane;

    float start_x = -1.0;
    float end_x = 1.0;
    float step_x = end_x-start_x / ((float)side_len);
    float start_y = -1.0;
    float end_y = 1.0;
    float step_y = end_y-start_y / ((float)side_len);

    for (int y=0; y != side_len; y++) {
        for (int x=0; x != side_len; x++) {
            plane.vertices.push_back({
                { start_x + ((float)x)*step_x, 0.0, start_y + ((float)y)*step_y },
                {
                    (y*side_len + x) % 3 == 0 ? 0.0 : 1.0,
                    (y*(side_len+1) + x) % 3 == 1 ? 0.0 : 1.0,
                    (y*(side_len+2) + x) % 3 == 2 ? 0.0 : 1.0,
                }
            });
        }
    }

    for (int y=0; y != side_len-1; y++) {
        for (int x=0; x != side_len-1; x++) {
            plane.indices.push_back(y*side_len + x);
            plane.indices.push_back(y*side_len + x + 1);
            plane.indices.push_back((y+1)*side_len + x);

            plane.indices.push_back((y+1)*side_len + x);
            plane.indices.push_back((y+1)*side_len + x + 1);
            plane.indices.push_back(y*side_len + x + 1);
        }
    }

    return plane;
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
