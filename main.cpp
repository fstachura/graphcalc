#include <glm/common.hpp>
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
#include "graph.hpp"
#include "mesh_object.hpp"
#include "expr_parser.hpp"

// NOTE: partially based on https://github.com/quazuo/grafika-mimuw

int fbWidth = 1200, fbHeight = 800;
bool fbSizeChanged = false;

struct GLScene {
    std::vector<std::shared_ptr<GLRenderable>> objects;
    GLCamera camera;

    void render() {
        for (auto&& r: objects) {
            r->render(camera);
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
    float camZoom = 0.0;
    double radius = 5.0;
    double movementSpeed = 0.5f;

    void tickInputEvents() {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            if (lastLeftButton) {
                double xoffset = xpos - lastX;
                double yoffset = ypos - lastY;

                xoffset *= mouseSensitivity;
                yoffset *= mouseSensitivity;

                if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
                    camX = glm::clamp(
                        camX + std::cos(camRx - glm::pi<double>() / 2.0) * xoffset +  std::cos(camRx) * yoffset,
                        -5.0, 5.0
                    );
                    camY = glm::clamp(
                        camY + std::sin(camRx - glm::pi<double>() / 2.0) * xoffset +  std::sin(camRx) * yoffset,
                        -5.0, 5.0
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

        glm::vec3 normDir = glm::normalize(scene.camera.where - scene.camera.position);
        scene.camera.position += camZoom * normDir;


        double normX = (2.0f*xpos)/fbWidth - 1.0f;
        double normY = 1.0f - (2.0f*ypos)/fbHeight;

        // https://antongerdelan.net/opengl/raycasting.html
        glm::vec3 ray_eye3 = glm::vec4(normX, normY, -1.0, 1.0) * glm::inverse(scene.camera.getProjectionMatrix());
        glm::vec4 ray_eye = glm::vec4(ray_eye3.x, ray_eye3.y, -1.0, 0.0);
        glm::vec4 ray_world4 = ray_eye * glm::inverse(scene.camera.getViewMatrix());
        glm::vec3 ray_world = glm::vec3(ray_world4.x, ray_world4.y, ray_world4.z);

        std::cout << "ray_world " << ray_world.x << " " << ray_world.y << " " << ray_world.z << std::endl;
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
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);

    if (!glfwInit()) {
        throw std::runtime_error("failed to init glfw");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
}

// TODO
// numbers on grid, z grid
// mouse raycasting

// TODO math

void fbSizeCallback(GLFWwindow *window, const int width, const int height) {
    if (width > 0 && height > 0) {
        fbWidth = width;
        fbHeight = height;
        fbSizeChanged = true;
    }
};

float zoomFactor = 0.0;

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    zoomFactor = glm::clamp(zoomFactor + yoffset, -10.0, 4.0);
};

int main() {
    initOpenGL();

    glfwWindowHint(GLFW_SAMPLES, 4);
    GLFWwindow* window = glfwCreateWindow(fbWidth, fbHeight, "graphcalc", nullptr, nullptr);
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
        std::cerr << "failed to init glew " << glew_init_result << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GL_TRUE);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto graph = std::make_shared<Graph>();

    std::shared_ptr<GLComputeShader> compute_shader = std::make_shared<GLComputeShader>();
    std::string minmax_shader = readFile("shaders/minmax.comp") + "float func(float x, float y) { return sin(x) * cos(y); }";
    compute_shader->setShader(minmax_shader);

    std::shared_ptr<GLShaderPipeline> grid_shaders = std::make_shared<GLShaderPipeline>();
    grid_shaders->setVertexShader(readFile("shaders/grid.vert"));
    grid_shaders->setFragmentShader(readFile("shaders/grid.frag"));

    std::shared_ptr<GLMeshObject> grid = std::make_shared<GLMeshObject>(generate_plane_mesh(11), grid_shaders);
    // grid->setWireframeMode(true);
    grid->setScale({3, 3, 3});
    // grid->setType(GLMeshObjectType::Line);

    App app { .window = window };
    app.scene.objects.push_back(graph);
    app.scene.objects.push_back(grid);

    // auto graph2 = std::make_shared<Graph>();
    // app.scene.objects.push_back(graph2);

    glfwSetWindowRefreshCallback(window, windowRefreshCallback);
    glfwSetWindowUserPointer(window, &app);

    glfwSetFramebufferSizeCallback(window, fbSizeCallback);
    glfwSetScrollCallback(window, scrollCallback);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    std::string error_str = "";
    char buf[1024] = {"sin(x)*cos(y)"};

    float center_x = 0, center_y = 0;
    float scale_x = 1.0, scale_y = 1.0, scale_z = 1.0;
    float tess_level = 16.0;
    bool wireframe_mode = false;
    bool dlight_enabled = false, plight_enabled = false;
    auto dlight = DirectionalLight {};
    auto plight = PointLight {};

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (!ImGui::GetIO().WantCaptureMouse)
            app.tickInputEvents();

        app.camZoom = zoomFactor;
        app.scene.render();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (ImGui::Begin("GraphCalc")) {
            if (ImGui::InputText("formula", buf, sizeof(buf))) {
                try {
                    graph->setCalcFunc(buf);
                    error_str = "";
                } catch (ParserError e) {
                    error_str = "Failed to parse: " + e.what + " in " + std::to_string(e.pos);
                } catch (TokenizerError e) {
                    error_str = "Failed to parse: " + e.what + " in " + std::to_string(e.pos);
                }
            }

            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", error_str.c_str());

            if (ImGui::DragFloat("center x", &center_x, 0.01f))
                graph->setCenter({center_x, center_y});
            if (ImGui::DragFloat("center y", &center_y, 0.01f))
                graph->setCenter({center_x, center_y});

            if (ImGui::DragFloat("scale x", &scale_x, 0.01f, 0.01, 100))
                graph->setScale({scale_x, scale_y, scale_z});
            if (ImGui::DragFloat("scale y", &scale_y, 0.01f, 0.01, 100))
                graph->setScale({scale_x, scale_y, scale_z});
            if (ImGui::DragFloat("scale z", &scale_z, 0.01f, 0.01, 100))
                graph->setScale({scale_x, scale_y, scale_z});

            ImGui::Text("approx. min %f max %f", graph->getLastMinMax().x, graph->getLastMinMax().y);

            if (ImGui::TreeNode("rendering settings")) {
                if (ImGui::Checkbox("wireframe mode", &wireframe_mode))
                    graph->setWireframeMode(wireframe_mode);

                if (ImGui::DragFloat("tesselation level", &tess_level, 0.1f, 2.0, 64.0))
                    graph->setTesselationLevel(tess_level);

                if (ImGui::TreeNode("point light")) {
                    if (ImGui::Checkbox("enabled", &plight_enabled))
                        graph->setPointLight(plight_enabled ?
                                std::optional<PointLight>({plight}) : std::optional<PointLight>());

                    if (ImGui::DragFloat("position x", &plight.position.x, 0.01f, -10.f, 10.f))
                        graph->setPointLight(plight_enabled ? std::optional<PointLight>({plight}) : std::optional<PointLight>());

                    if (ImGui::DragFloat("position y", &plight.position.y, 0.01f, -10.f, 10.f))
                        graph->setPointLight(plight_enabled ? std::optional<PointLight>({plight}) : std::optional<PointLight>());

                    if (ImGui::DragFloat("position z", &plight.position.z, 0.01f, -10.f, 10.f))
                        graph->setPointLight(plight_enabled ? std::optional<PointLight>({plight}) : std::optional<PointLight>());

                    if (ImGui::DragFloat("att quad", &plight.att_quadratic, 0.01f, -10.f, 10.f))
                        graph->setPointLight(plight_enabled ? std::optional<PointLight>({plight}) : std::optional<PointLight>());

                    if (ImGui::DragFloat("att lin", &plight.att_linear, 0.01f, -10.f, 10.f))
                        graph->setPointLight(plight_enabled ? std::optional<PointLight>({plight}) : std::optional<PointLight>());

                    if (ImGui::DragFloat("att const", &plight.att_constant, 0.01f, -10.f, 10.f))
                        graph->setPointLight(plight_enabled ? std::optional<PointLight>({plight}) : std::optional<PointLight>());

                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("directional light")) {
                    if (ImGui::Checkbox("enabled", &dlight_enabled))
                        graph->setDirectionalLight(dlight_enabled ?
                                std::optional<DirectionalLight>({dlight}) : std::optional<DirectionalLight>());

                    if (ImGui::DragFloat("direction x", &dlight.direction.x, 0.01f, -10.f, 10.f))
                        graph->setDirectionalLight(dlight_enabled ?
                                std::optional<DirectionalLight>({dlight}) : std::optional<DirectionalLight>());

                    if (ImGui::DragFloat("direction y", &dlight.direction.y, 0.01f, -10.f, 10.f))
                        graph->setDirectionalLight(dlight_enabled ?
                                std::optional<DirectionalLight>({dlight}) : std::optional<DirectionalLight>());

                    if (ImGui::DragFloat("direction z", &dlight.direction.z, 0.01f, -10.f, 10.f))
                        graph->setDirectionalLight(dlight_enabled ?
                                std::optional<DirectionalLight>({dlight}) : std::optional<DirectionalLight>());

                    ImGui::TreePop();
                }

                ImGui::TreePop();
            }
        }

        ImGui::End();

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

        glFinish();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
