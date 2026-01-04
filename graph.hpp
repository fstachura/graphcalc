#pragma once
#include <memory>

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include "GL/glew.h"

#include "shader_pipeline.hpp"
#include "mesh_object.hpp"
#include "utils.hpp"
#include "expr_parser.hpp"

class Graph: public GLRenderable {
    glm::vec2 center = {0.0, 0.0};
    glm::vec2 rangeX = {-1.0, 1.0};
    glm::vec2 rangeY = {-1.0, 1.0};
    std::unique_ptr<GLMeshObject> plane;
    std::shared_ptr<GLShaderPipeline> shaders;
    std::string tessEvalShader;

public:
    Graph() {
        shaders = std::make_shared<GLShaderPipeline>();
        shaders->setVertexShader(readFile("shaders/plane.vert"));
        shaders->setFragmentShader(readFile("shaders/plane.frag"));
        shaders->setTessCtrlShader(readFile("shaders/plane.tesc"));
        tessEvalShader = readFile("shaders/plane.tese");
        std::string calcFunc = "float func(float x, float y) { return sin(x) * cos(y); }";
        shaders->setTessEvalShader(tessEvalShader + calcFunc);
        shaders->setPatchVertices(3);

        plane = std::make_unique<GLMeshObject>(generate_plane_mesh(128), shaders);
        plane->setTesselation(true);
        plane->setScale({3, 3, 3});
    }

    virtual void render(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix) override {
        plane->getShaders().enable();
        plane->getShaders().setUniform("center", center);
        plane->getShaders().setUniform("rangeX", rangeX);
        plane->getShaders().setUniform("rangeY", rangeY);
        plane->render(viewMatrix, projectionMatrix);
    }

    void setCalcFunc(std::string func) {
        std::string calcFunc = Parser(tokenize(func)).parse()->to_string();

        calcFunc = "float func(float x, float y) { return float(" + calcFunc + "); }";
        std::cout << calcFunc << std::endl;
        shaders->setTessEvalShader(tessEvalShader + calcFunc);
    }

    void setCenter(glm::vec2 center) {
        this->center = center;
    }

    void setRangeX(glm::vec2 rangeX) {
        this->rangeX = rangeX;
    }

    void setRangeY(glm::vec2 rangeY) {
        this->rangeY = rangeY;
    }

    virtual ~Graph() { }
};
