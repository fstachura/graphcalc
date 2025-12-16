#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
#include <filesystem>

#include <glm/glm.hpp>
// extension wrangler, runtime mechanisms for determining which extensions are supported
#include "GL/glew.h"
// wraps OpenGL, provides APIs for easy initialization, input handling, multiple monitors...
#include "GLFW/glfw3.h"
// OpenGL mathematics library

#include "imgui.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "utils.hpp"
#include "shader_pipeline.hpp"
#include "mesh_object.hpp"

// NOTE: partially based on https://github.com/quazuo/grafika-mimuw

class GLCamera {
public:
    glm::vec3 position { 0, 0, 0 };
    glm::vec3 up { 0, 1, 0 };
    glm::vec3 where { 0, 0, 0 };
    float aspectRatio = 4.05 / 3.0f;
    float fieldOfView = 80.f;
    float zNear = 0.1f;
    float zFar = 500.f;

    GLCamera() {
    }

    glm::mat4 getViewMatrix() {
        // TODO math behind this
        return glm::lookAt(position, where, up);
    }

    glm::mat4 getProjectionMatrix() {
        return glm::perspective(glm::radians(fieldOfView), aspectRatio, zNear, zFar);
    }
};

struct GLScene {
    std::vector<std::unique_ptr<GLRenderable>> objects;
    GLCamera camera;

    void render() {
        for (auto&& r: objects) {
            r->render(camera.getViewMatrix(), camera.getProjectionMatrix());
        }
    }
};

struct App {
    GLScene scene;
    GLFWwindow* window;
    bool lastLeftButton = false;
    double lastX = 600, lastY = 400;
    double mouseSensitivity = 0.01f;
    double camRx = 0.0;
    double camRy = 0.0;
    double camX = 0.0;
    double camY = 0.0;
    double radius = 50.0;
    double movementSpeed = 0.5f;

    void tickInputEvents() {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);

            if (lastLeftButton) {
                double xoffset = xpos - lastX;
                double yoffset = ypos - lastY;

                xoffset *= mouseSensitivity;
                yoffset *= mouseSensitivity;

                if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
                    camX = glm::clamp(
                        camX + std::cos(camRx - glm::pi<double>() / 2.0) * xoffset +  std::cos(camRx) * yoffset,
                        -50.0, 50.0
                    );
                    camY = glm::clamp(
                        camY + std::sin(camRx - glm::pi<double>() / 2.0) * xoffset +  std::sin(camRx) * yoffset,
                        -50.0, 50.0
                    );
                } else {
                    camRx += xoffset;
                    camRy = glm::clamp(
                        camRy + yoffset,
                        -glm::pi<double>() / 2.0 + 0.001,
                        glm::pi<double>() / 2.0 - 0.001
                    );
                }
            }

            lastX = xpos;
            lastY = ypos;
            lastLeftButton = true;
        } else {
            lastLeftButton = false;
        }

        scene.camera.where.x = camX;
        scene.camera.where.z = camY;
        scene.camera.position = scene.camera.where + glm::vec3 {
            cos(camRx) * cos(camRy) * radius,
            sin(camRy) * radius,
            sin(camRx) * cos(camRy) * radius,
        };

        // std::cout
        //     << camX << " " << camY << " "
        //     << scene.camera.position.x << " " 
        //     << scene.camera.position.y << " " 
        //     << scene.camera.position.z << " " 
        //     << std::endl;
    }
};

void framebufferSizeCallback(GLFWwindow *window, const int width, const int height) {
    if (width > 0 && height > 0) {
        glViewport(0, 0, width, height);
    }
}

void windowRefreshCallback(GLFWwindow *window) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    app->scene.render();
    glfwSwapBuffers(window);
    glFinish();
}

void initOpenGL() {
    if (!glfwInit()) {
        throw std::runtime_error("failed to init glfw");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
}

// TODO
// coordinates? size of the plane
// handle window resizing
// add lightning and shadows
// parse expression from GUI
// add optional texture and other options from GUI
// polar coordinates?

// rotate by mouse - kinda works, TODO math

// +,-,*,/,sin,cos,tan,tanh,ctan,ctanh,pow,sqrt,exp
// pi, e

// parse into tree -> sequence of stack-based instructions -> 

int main() {
    initOpenGL();

    GLFWwindow* window = glfwCreateWindow(1200, 800, "graphcalc", nullptr, nullptr);
    if (!window) {
        const char* desc;
        std::cerr << "failed to open glfw window " << glfwGetError(&desc) << " " << desc << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    glewExperimental = true;
    auto glew_init_result = glewInit();
    if (glew_init_result != GLEW_OK) {
        std::cerr << "failed to init glew" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GL_TRUE);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);

    std::shared_ptr<GLShaderPipeline> shaders = std::make_shared<GLShaderPipeline>();
    // shaders->setVertexShader(readFile("shaders/main.vert"));
    // shaders->setFragmentShader(readFile("shaders/main.frag"));
    shaders->setVertexShader(readFile("shaders/plane.vert"));
    shaders->setFragmentShader(readFile("shaders/plane.frag"));
    shaders->setTessCtrlShader(readFile("shaders/plane.tesc"));
    std::string tessEvalShader = readFile("shaders/plane.tese");
    std::string calcFunc = "float func(float x, float y) { return sin(x) + cos(y); }";
    shaders->setTessEvalShader(tessEvalShader + calcFunc);

    shaders->setPatchVertices(3);
    std::unique_ptr<GLMeshObject> plane = std::make_unique<GLMeshObject>(generate_plane_mesh(128), shaders);

    App app { .window = window };
    app.scene.objects.push_back(std::move(plane));

    glfwSetWindowRefreshCallback(window, windowRefreshCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetWindowUserPointer(window, &app);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplGlfw_InitForOpenGL(window, true); 
    ImGui_ImplOpenGL3_Init();

    char buf[512] = {0};
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        app.tickInputEvents();
        app.scene.render();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (ImGui::Begin("GraphCalc")) {
            if (ImGui::InputText("formula", buf, sizeof(buf))) {
                std::string calcFunc(buf);
                calcFunc = "float func(float x, float y) { return float(" + calcFunc + "); }";

                GLuint shaderID = glCreateShader(GL_TESS_EVALUATION_SHADER);
                char const *shaderSrc = calcFunc.c_str();
                glShaderSource(shaderID, 1, &shaderSrc, nullptr);
                glCompileShader(shaderID);
                bool ok = false;
                try {
                    checkShader(shaderID);
                    ok = true;
                } catch (std::runtime_error& e) {
                    std::cout << "failed to compile expression: " << std::endl;
                    std::cout << e.what() << std::endl;
                }
                glDeleteShader(shaderID);

                if (ok) {
                    shaders->setTessEvalShader(tessEvalShader + calcFunc);
                }
            }
            ImGui::End();
        }

        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
