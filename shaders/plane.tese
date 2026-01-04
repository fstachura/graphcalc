#version 410 core

// https://www.ogldev.org/www/tutorial30/tutorial30.html

layout (triangles, equal_spacing, ccw) in;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 center;

in vec3 in_color[];
in vec3 in_position[];

out vec3 position;
out vec3 color;

#define pi 3.14159265358979323846lf
#define e  2.7182818284590452354lf

float func(float x, float y);

vec3 interpolate3D(vec3 a, vec3 b, vec3 c) {
    return a * vec3(gl_TessCoord.x) + b * vec3(gl_TessCoord.y) + c * vec3(gl_TessCoord.z);
}

double gc_sin(double x) {
    return double(sin(float(x)));
}

double gc_cos(double x) {
    return double(cos(float(x)));
}

double gc_tan(double x) {
    return double(tan(float(x)));
}

double gc_asin(double x) {
    return double(asin(float(x)));
}

double gc_acos(double x) {
    return double(acos(float(x)));
}

double gc_atan(double x) {
    return double(atan(float(x)));
}

double gc_sinh(double x) {
    return double(sinh(float(x)));
}

double gc_cosh(double x) {
    return double(cosh(float(x)));
}

double gc_tanh(double x) {
    return double(tanh(float(x)));
}

double gc_asinh(double x) {
    return double(asinh(float(x)));
}

double gc_acosh(double x) {
    return double(acosh(float(x)));
}

double gc_atanh(double x) {
    return double(atanh(float(x)));
}

double gc_exp(double x) {
    return double(exp(float(x)));
}

double gc_log(double x) {
    return double(log(float(x)));
}

double gc_exp2(double x) {
    return double(exp2(float(x)));
}

double gc_log2(double x) {
    return double(log2(float(x)));
}

double gc_pow(double x, double y) {
    return double(pow(float(x), float(y)));
}

void main() {
    position = interpolate3D(gl_in[0].gl_Position.xyz, gl_in[1].gl_Position.xyz, gl_in[2].gl_Position.xyz);
    position.y = func(position.x + center.x, position.z + center.y);

    color = interpolate3D(in_color[0], in_color[1], in_color[2]);
    gl_Position = projection * view * model * vec4(position, 1.0);
    //gl_Position = vec4(position[0], 1.0);
}

