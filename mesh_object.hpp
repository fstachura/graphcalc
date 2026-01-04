#pragma once
#include <memory>
#include <vector>

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include "GL/glew.h"

#include "vertex.hpp"
#include "shader_pipeline.hpp"
#include "utils.hpp"

class GLMeshObject: public GLRenderable {
    // vertex array object - storespalące mnie pytanie calls to glEnableVertexAttribArray, vertex attribute configurations (glVertexAttribPointer) and vertex buffer objects associated with vertex attributes by calls to glVertexAttribPointer
    GLuint vao;
    // vertex buffer object - stores vertices
    GLuint vbo;
    // element buffer - stores vertex indices that OpenGL uses to decide what vertices to draw
    GLuint ebo;

    float center_x = 0;
    float center_y = 0;
    bool wireframe_mode = false;
    bool tesselation = false;

    std::shared_ptr<GLShaderPipeline> shaderPipeline;
    GLMesh mesh;

public:
    GLMeshObject(GLMesh mesh, std::shared_ptr<GLShaderPipeline> shaderPipeline): mesh(mesh), shaderPipeline{shaderPipeline} {
        // create vertex array
        // number of vertex array objects, array where array names are stored
        // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGenVertexArrays.xhtml
        glGenVertexArrays(1, &vao);
        // bind vertex array object
        // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBindVertexArray.xhtml
        glBindVertexArray(vao);

        // create a vertex buffer
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        // copy data to buffer
        // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBufferData.xhtml
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * mesh.vertices.size(),
                mesh.vertices.data(), GL_STATIC_DRAW);

        // create an element buffer
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh.indices.size(),
                mesh.indices.data(), GL_STATIC_DRAW);
        // GL_STATIC_DRAW - data is set once and used many times (DYNAMIC - changed a lot and used many times, STREAM - set once and used few times)

        // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glVertexAttribPointer.xhtml
        // specify location and data format of the array of generic vertex attributes
        glVertexAttribPointer(
            0, // vertex attribute to be modified (location = 0 in shader layout)
            3, // components per generic vertex attribute
            GL_FLOAT, // data type of each component in the array
            GL_FALSE, // normalize (map to [-1,1] or [0,1] for unsigned) fixed-point values?
            sizeof(Vertex), // stride - offset between consecutive generic attributes
            reinterpret_cast<void *>(offsetof(Vertex, position)) // pointer to the first component of the generic vertex attribute
        );
        // enable generic vertex attribute array
        glEnableVertexAttribArray(0);

        // vertex attrib pointer for color
        glVertexAttribPointer(
            1,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            reinterpret_cast<void *>(offsetof(Vertex, color))
        );
        glEnableVertexAttribArray(1);
    }

    void set_center_x(float x) {
        center_x = x;
    }

    void set_center_y(float y) {
        center_y = y;
    }

    void set_wireframe_mode(bool wireframe_mode) {
        this->wireframe_mode = wireframe_mode;
    }

    void set_tesselation(bool tesselation) {
        this->tesselation = tesselation;
    }

    void render(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix) override {
        shaderPipeline->enable();

        // translation matrix
        shaderPipeline->setUniform("model", glm::translate(
            glm::mat4(1.0f),
            glm::vec3(-60.0f, 0.0f, -60.0f)
        ));
        shaderPipeline->setUniform("view", viewMatrix);
        shaderPipeline->setUniform("projection", projectionMatrix);
        shaderPipeline->setUniform("center", glm::vec2{center_x, center_y});

        // wireframe mode
        if (this->wireframe_mode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        if (tesselation) {
            glDrawElements(GL_PATCHES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
        } else {
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
        }
    }

    virtual ~GLMeshObject() {
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &ebo);
    }
};
