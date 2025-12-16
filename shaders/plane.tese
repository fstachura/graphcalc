#version 410 core

// https://www.ogldev.org/www/tutorial30/tutorial30.html

layout (triangles, equal_spacing, ccw) in;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

in vec3 in_color[];
in vec3 in_position[];

out vec3 position;
out vec3 color;

float func(float x, float y);

vec3 interpolate3D(vec3 a, vec3 b, vec3 c) {
    return a * vec3(gl_TessCoord.x) + b * vec3(gl_TessCoord.y) + c * vec3(gl_TessCoord.z);
}

void main() {
    position = interpolate3D(gl_in[0].gl_Position.xyz, gl_in[1].gl_Position.xyz, gl_in[2].gl_Position.xyz);
    position.y = func(position.x, position.z);

    color = interpolate3D(in_color[0], in_color[1], in_color[2]);
    gl_Position = projection * view * model * vec4(position, 1.0);
    //gl_Position = vec4(position[0], 1.0);
}

