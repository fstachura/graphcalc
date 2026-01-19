#pragma once
#include <GL/gl.h>
#include <memory>
#include <optional>
#include <vector>
#include <algorithm>

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include "GL/glew.h"

#include "shader_pipeline.hpp"
#include "mesh_object.hpp"
#include "utils.hpp"
#include "expr_parser.hpp"

class MinMax {
    glm::vec2 center = {0.0, 0.0};
    glm::vec3 scale  = {1.0, 1.0, 1.0};

    std::shared_ptr<GLComputeShader> shader;
    std::string calcFunc = "float func(float x, float y) { return sin(x) * cos(y); }";
    std::string minmaxShader = readFile("shaders/minmax.comp");

    unsigned int minBuffer;
    unsigned int maxBuffer;

    unsigned int height;
    unsigned int width; 

    std::vector<float> min;
    std::vector<float> max;

public:
    MinMax(int width=100, int height=100): width(width), height(height) {
        shader = std::make_shared<GLComputeShader>();
        shader->setShader(minmaxShader + calcFunc);

        minBuffer = createGLBuffer(sizeof(float)*height*width);
        maxBuffer = createGLBuffer(sizeof(float)*height*width);

        min = std::vector<float>(width*height, -1.0);
        max = std::vector<float>(width*height, 1.0);
    }

    void setCalcFuncRaw(std::string func) {
        // TODO does calculating in doubles and casting to floats make any sense?
        calcFunc = "float func(float x, float y) { return float(" + func + "); }";
        std::cout << calcFunc << std::endl;
        shader->setShader(minmaxShader + calcFunc);
    }

    glm::vec2 calculate() {
        shader->enable();

        shader->setUniform("center", center);
        shader->setUniform("scale", scale);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, minBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, maxBuffer);

        glDispatchCompute(width, height, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glGetNamedBufferSubData(minBuffer, 0, width*height*sizeof(float), min.data());
        glGetNamedBufferSubData(maxBuffer, 0, width*height*sizeof(float), max.data());

        // std::cout<<"max"<<std::endl;
        // for (auto&& el: max)
        //     std::cout << el << " ";
        // std::cout << std::endl;

        // std::cout<<"min"<<std::endl;
        // for (auto&& el: min)
        //     std::cout << el << " ";
        // std::cout << std::endl;

        return glm::vec2(
            *std::min_element(min.begin(), min.end()),
            *std::max_element(max.begin(), max.end())
        );
    }

    void setCenter(glm::vec2 center) {
        this->center = center;
    }

    void setScale(glm::vec3 scale) {
        this->scale = scale;
    }

    ~MinMax() {
        glDeleteBuffers(1, &minBuffer);
        glDeleteBuffers(1, &maxBuffer);
    }
};

class Graph: public GLRenderable {
    glm::vec2 center = {0.0, 0.0};
    glm::vec2 rangeX = {-1.0, 1.0};
    glm::vec2 rangeY = {-1.0, 1.0};
    glm::vec2 minMax = {-1.0, 1.0};
    glm::vec3 scale  = {1.0, 1.0, 1.0};
    unsigned int colormapTexture;
    bool lighting = false;

    std::optional<DirectionalLight> directionalLight;
    std::optional<PointLight> pointLight;

    std::unique_ptr<GLMeshObject> plane;
    std::shared_ptr<GLShaderPipeline> shaders;
    std::string tessEvalShader;

    MinMax minMaxCalc;

    void recalculateMinMax() {
        minMaxCalc.setCenter(center);
        minMaxCalc.setScale(scale);
        minMax = minMaxCalc.calculate();
    }

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
        plane->setType(GLMeshObjectType::Tesselated);
        plane->setScale({3, 3, 3});

        colormapTexture = create1DGLTexture("plasma_colormap.png");
    }

    virtual void render(const GLCamera& camera) override {
        plane->getShaders().enable();
        plane->getShaders().setUniform("center", center);
        plane->getShaders().setUniform("scale", scale);
        plane->getShaders().setUniform("minMax", minMax);

        auto dlight = directionalLight.value_or(DirectionalLight {});
        plane->getShaders().setUniform("directional_light_enabled", directionalLight.has_value() ? 1 : 0);
        plane->getShaders().setUniform("directional_light.direction", glm::normalize(dlight.direction));
        plane->getShaders().setUniform("directional_light.color", dlight.color);

        auto plight = pointLight.value_or(PointLight {});
        plane->getShaders().setUniform("point_light_enabled", pointLight.has_value() ? 1 : 0);
        plane->getShaders().setUniform("point_light.position", glm::normalize(plight.position));
        plane->getShaders().setUniform("point_light.color", dlight.color);
        plane->getShaders().setUniform("point_light.att_constant", plight.att_constant);
        plane->getShaders().setUniform("point_light.att_linear", plight.att_linear);
        plane->getShaders().setUniform("point_light.att_quadratic", plight.att_quadratic);

        glBindTexture(GL_TEXTURE_1D, colormapTexture);
        plane->render(camera);
    }

    void setCalcFunc(std::string func) {
        std::string calcFunc = Parser(tokenize(func)).parse()->to_string();
        minMaxCalc.setCalcFuncRaw(calcFunc);

        // TODO does calculating in doubles and casting to floats make any sense?
        calcFunc = "float func(float x, float y) { return float(" + calcFunc + "); }";
        // std::cout << calcFunc << std::endl;
        shaders->setTessEvalShader(tessEvalShader + calcFunc);

        recalculateMinMax();
    }

    void setCenter(glm::vec2 center) {
        this->center = center;
        recalculateMinMax();
    }

    void setScale(glm::vec3 scale) {
        this->scale = scale;
        recalculateMinMax();
    }

    void setTesselationLevel(float tessLevel) {
        plane->setTesselationLevel(tessLevel);
    }

    void setWireframeMode(bool wireframeMode) {
        plane->setWireframeMode(wireframeMode);
    }

    void setPointLight(std::optional<PointLight> plight) {
        this->pointLight = plight;
    }

    void setDirectionalLight(std::optional<DirectionalLight> dlight) {
        this->directionalLight = dlight;
    }

    glm::vec2 getLastMinMax() {
        return minMax;
    }

    virtual ~Graph() {
        glDeleteTextures(1, &colormapTexture);
    }
};
