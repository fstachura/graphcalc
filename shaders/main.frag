#version 410

in vec3 color;
in vec3 position;

out vec4 out_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    out_color = vec4(
        mix(vec3(0.0, 0.0, 1.0), vec3(1.0, 1.0, 0.0), sin(position.y)),
        1.0f
    );
    // out_color = vec4(color, 1.0f);
}
