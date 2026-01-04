#version 410

in vec3 color;
in vec3 position;

out vec4 out_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 center;

void main() {
    out_color = vec4(
        vec3(0.5, 0.5, 0.5),
        1.0f
    );
}
