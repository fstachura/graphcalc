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
#include "expr_parser.hpp"

// NOTE: partially based on https://github.com/quazuo/grafika-mimuw

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

    GLCamera() {
    }

    glm::mat4 getViewMatrix() {
        // TODO math behind this
        return glm::lookAt(position, where, up);
    }

    glm::mat4 getProjectionMatrix() {
        return glm::perspective(glm::radians(fieldOfView), aspectRatio, zNear, zFar);
    }

    void setAspectRatio(float aspectRatio) {
        this->aspectRatio = aspectRatio;
    }
};

struct GLScene {
    std::vector<std::shared_ptr<GLRenderable>> objects;
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
// add optional texture and other options from GUI
// polar coordinates?

// rotate by mouse - kinda works, TODO math

int fbWidth = 1200, fbHeight = 800;
bool fbSizeChanged = false;

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

    std::shared_ptr<GLMeshObject> plane = std::make_shared<GLMeshObject>(generate_plane_mesh(128), shaders);
    plane->set_tesselation(true);

    std::shared_ptr<GLShaderPipeline> grid_shaders = std::make_shared<GLShaderPipeline>();
    grid_shaders->setVertexShader(readFile("shaders/grid.vert"));
    grid_shaders->setFragmentShader(readFile("shaders/grid.frag"));

    std::shared_ptr<GLMeshObject> grid = std::make_shared<GLMeshObject>(generate_plane_mesh(128), grid_shaders);
    grid->set_wireframe_mode(true);

    App app { .window = window };
    app.scene.objects.push_back(plane);
    app.scene.objects.push_back(grid);

    glfwSetWindowRefreshCallback(window, windowRefreshCallback);
    glfwSetWindowUserPointer(window, &app);

    auto fbSizeCallback = [](GLFWwindow *window, const int width, const int height) {
        if (width > 0 && height > 0) {
            fbWidth = width;
            fbHeight = height;
            fbSizeChanged = true;
        }
    };

    glfwSetFramebufferSizeCallback(window, fbSizeCallback);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    std::string error_str = "";
    char buf[1024] = {0};

    float center_x = 0;
    float center_y = 0;

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (!ImGui::GetIO().WantCaptureMouse)
            app.tickInputEvents();

        app.scene.render();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (ImGui::Begin("GraphCalc")) {
            if (ImGui::InputText("formula", buf, sizeof(buf))) {
                try {
                    std::string calcFunc = Parser(tokenize(buf)).parse()->to_string();

                    calcFunc = "float func(float x, float y) { return float(" + calcFunc + "); }";
                    std::cout << calcFunc << std::endl;
                    shaders->setTessEvalShader(tessEvalShader + calcFunc);

                    error_str = "";
                } catch (ParserError e) {
                    error_str = "Failed to parse: " + e.what + " in " + std::to_string(e.pos);
                } catch (TokenizerError e) {
                    error_str = "Failed to parse: " + e.what + " in " + std::to_string(e.pos);
                }
            }

            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", error_str.c_str());

            if (ImGui::DragFloat("center x", &center_x, 0.01f))
                plane->set_center_x(center_x);

            if (ImGui::DragFloat("center y", &center_y, 0.01f))
                plane->set_center_y(center_y);

            ImGui::End();
        }

        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (fbSizeChanged) {
            glViewport(0, 0, fbWidth, fbHeight);
            app.scene.camera.setAspectRatio((float)fbWidth/(float)fbHeight);
            fbSizeChanged = false;
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
