#version 410

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_color;

out vec3 color;
out vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 center;

void main() {
    position = in_position;
    gl_Position = projection * view * model * vec4(position, 1.0);
    color = in_color;
}
