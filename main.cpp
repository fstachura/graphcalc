#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
#include <map>
#include <filesystem>
#include <fstream>

// extension wrangler, runtime mechanisms for determining which extensions are supported
#include "GL/glew.h"
// wraps OpenGL, provides APIs for easy initialization, input handling, multiple monitors...
#include "GLFW/glfw3.h"
// OpenGL mathematics library
#include <glm/glm.hpp>

// NOTE: partially based on https://github.com/quazuo/grafika-mimuw

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
};

static const float X = 0.525731112119133606;
static const float Z = 0.850650808352039932;

const std::vector<Vertex> tetrahedron_vertices{
    {{ -X, 0.0,   Z}, {1.0, 0.0, 0.0}},
    {{  X, 0.0,   Z}, {0.0, 1.0, 0.0}},
    {{ -X, 0.0,  -Z}, {0.0, 0.0, 1.0}},
    {{  X, 0.0,  -Z}, {1.0, 1.0, 0.0}},
    {{0.0,   Z,   X}, {1.0, 0.0, 1.0}},
    {{0.0,   Z,  -X}, {0.0, 1.0, 1.0}},
    {{0.0,  -Z,   X}, {1.0, 0.0, 0.0}},
    {{0.0,  -Z,  -X}, {0.0, 1.0, 0.0}},
    {{  Z,   X, 0.0}, {0.0, 0.0, 1.0}},
    {{ -Z,   X, 0.0}, {1.0, 1.0, 0.0}},
    {{  Z,  -X, 0.0}, {1.0, 0.0, 1.0}},
    {{ -Z,  -X, 0.0}, {0.0, 1.0, 1.0}},
};

const std::vector<GLuint> tetrahedron_indices{
    0, 4, 1,
    0, 9, 4,
    9, 5, 4,
    4, 5, 8,
    4, 8, 1,
    8, 10, 1,
    8, 3, 10,
    5, 3, 8,
    5, 2, 3,
    2, 7, 3,
    7, 10, 3,
    7, 6, 10,
    7, 11, 6,
    11, 0, 6,
    0, 1, 6,
    6, 1, 10,
    9, 0, 11,
    9, 11, 2,
    9, 2, 5,
    7, 2, 11
};

void framebufferSizeCallback(GLFWwindow *window, const int width, const int height) {
    if (width > 0 && height > 0) {
        glViewport(0, 0, width, height);
    }
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

class GLShaders {
    GLuint id;
    std::map<std::string, GLint> uniformIDs {};

public:
    GLShaders(const std::filesystem::path &vertexShaderPath, const std::filesystem::path &fragmentShaderPath) {
        id = glCreateProgram();
        const GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderPath);
        const GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderPath);
        linkProgram(vertexShader, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    ~GLShaders() {
        glDeleteProgram(id);
    }

    void enable() const {
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

    GLint getUniformID(const std::string &name) {
        const auto it = uniformIDs.find(name);
        if (it != uniformIDs.end()) {
            return it->second;
        }

        const GLint uniformId = glGetUniformLocation(id, name.c_str());
        if (uniformId == -1) {
            throw std::runtime_error("failed to get uniform with name: " + name);
        }

        uniformIDs.emplace(name, uniformId);
        return uniformId;
    }

    GLuint compileShader(const GLuint shaderKind, const std::filesystem::path &path) const {
        GLuint shaderID = glCreateShader(shaderKind);
        std::string shaderCode = readFile(path);

        std::cout << "Compiling shader: " << path.filename() << "\n";
        char const *vertexSourcePointer = shaderCode.c_str();
        // shader id, how many code strings, actual code, array of string lengths
        glShaderSource(shaderID, 1, &vertexSourcePointer, nullptr);
        glCompileShader(shaderID);

        checkShader(shaderID);
        return shaderID;
    }

    void linkProgram(const GLuint vertexShader, const GLuint fragmentShader) const {
        glAttachShader(id, vertexShader);
        glAttachShader(id, fragmentShader);
        glLinkProgram(id);
        checkProgram(id);
    }
};

// architecutre:
// mesh (vertices + indices)
// uniforms (map) - that makes no sense, uniform almost doesn't store anything
// shader (code + pointer to uniform)
// shader program (shaders)
// mesh object (buffers, shaders)
// renderer (list of objects, camera position)

struct GLMesh {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
};

class GLRenderer {
    // vertex array object - storespalące mnie pytanie calls to glEnableVertexAttribArray, vertex attribute configurations (glVertexAttribPointer) and vertex buffer objects associated with vertex attributes by calls to glVertexAttribPointer
    GLuint vao;
    // vertex buffer object - stores vertices
    GLuint vbo;
    // element buffer - stores vertex indices that OpenGL uses to decide what vertices to draw
    GLuint ebo;

    std::unique_ptr<GLShaders> shaders;
    GLMesh mesh;

public:
    GLRenderer(GLMesh mesh): mesh(mesh) {
        shaders = std::make_unique<GLShaders>("shaders/main.vert", "shaders/main.frag");

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

    void render(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix) {
        shaders->enable();

        shaders->setUniform("model", glm::identity<glm::mat4>());
        shaders->setUniform("view", viewMatrix);
        shaders->setUniform("projection", projectionMatrix);

        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
    }

    ~GLRenderer() {
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &ebo);
    }
};

void windowRefreshCallback(GLFWwindow *window) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    GLRenderer* renderer = static_cast<GLRenderer*>(glfwGetWindowUserPointer(window));
    // TODO
    // renderer->render();
    glfwSwapBuffers(window);
    glFinish();
}

void initOpenGL() {
    if (!glfwInit()) {
        throw std::runtime_error("failed to init glfw");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
}

std::pair<glm::vec3, glm::vec2> tickInputEvents(GLFWwindow* window, glm::vec3 cameraPosition, glm::vec2 cameraRotation) {
    // TODO
    const glm::vec3 front = {
        std::cos(cameraRotation.y) * std::sin(cameraRotation.x),
        std::sin(cameraRotation.y),
        std::cos(cameraRotation.y) * std::cos(cameraRotation.x)
    };

    const glm::vec3 right = glm::vec3(
        std::sin(cameraRotation.x - 3.14f / 2.0f),
        0,
        std::cos(cameraRotation.x - 3.14f / 2.0f)
    );

    // cross product
    const glm::vec3 up = glm::cross(right, front);
}

class GLCamera {
    glm::vec3 cameraPosition { 0, 0, -3 };
    glm::vec2 cameraRotation { 0, 0 };
    float aspectRatio = 4.05 / 3.0f;
    float fieldOfView = 80.f;
    float zNear = 0.1f;
    float zFar = 500.f;

public:
    GLCamera() {
    }

    glm::mat4 getViewMatrix() {
        const glm::vec3 front = {
            std::cos(cameraRotation.y) * std::sin(cameraRotation.x),
            std::sin(cameraRotation.y),
            std::cos(cameraRotation.y) * std::cos(cameraRotation.x)
        };

        return glm::lookAt(cameraPosition, cameraPosition + front, glm::vec3(0, 1, 0));
    }

    glm::mat4 getProjectionMatrix() {
        return glm::perspective(glm::radians(fieldOfView), aspectRatio, zNear, zFar);
    }
};

GLMesh generate_plane_mesh(int side_len) {
    GLMesh plane;

    float start_x = -X;
    float end_x = X;
    float step_x = end_x-start_x / ((float)side_len);
    float start_y = -Z;
    float end_y = Z;
    float step_y = end_y-start_y / ((float)side_len);

    for (int y=0; y != side_len; y++) {
        for (int x=0; x != side_len; x++) {
            plane.vertices.push_back({
                { start_x + ((float)x)*step_x, start_y + ((float)y)*step_y, 0.0 },
                {
                    (y*side_len + x) % 3 == 0 ? 0.0 : 1.0,
                    (y*side_len + x) % 3 == 1 ? 0.0 : 1.0,
                    (y*side_len + x) % 3 == 2 ? 0.0 : 1.0,
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

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);

    GLRenderer renderer(generate_plane_mesh(16));
    // GLMesh tetrahedron { tetrahedron_vertices, tetrahedron_indices };
    // GLRenderer renderer(tetrahedron);

    glfwSetWindowRefreshCallback(window, windowRefreshCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetWindowUserPointer(window, &renderer);

    GLCamera camera;

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderer.render(camera.getViewMatrix(), camera.getProjectionMatrix());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

