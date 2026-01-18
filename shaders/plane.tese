#version 410 core

// https://www.ogldev.org/www/tutorial30/tutorial30.html

precision highp float;

layout (triangles, equal_spacing, ccw) in;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 center;
uniform vec3 scale;
uniform vec2 minMax;
uniform sampler1D colormap;

in vec3 in_color[];
in vec3 in_position[];

out vec3 position;
out vec3 color;

float func(float x, float y);

vec3 interpolate3D(vec3 a, vec3 b, vec3 c) {
    return a * vec3(gl_TessCoord.x) + b * vec3(gl_TessCoord.y) + c * vec3(gl_TessCoord.z);
}

// GC MATH - keep in sync with other shaders

#define pi 3.14159265358979323846lf
#define e  2.7182818284590452354lf

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

// END GC MATH

highp vec2 map_position(highp vec2 position, highp vec3 scale) {
    highp vec2 scaled_position = (position+1.0f) / 2.0f;

    return vec2(
        scale.x * scaled_position.x - scale.x/2,
        scale.y * scaled_position.y - scale.y/2
    );
}

void main() {
    position = interpolate3D(gl_in[0].gl_Position.xyz, gl_in[1].gl_Position.xyz, gl_in[2].gl_Position.xyz);

    vec2 mapped_position = map_position(vec2(position.x, position.z), scale);
    float result = func(mapped_position.x + center.x, mapped_position.y + center.y) * scale.z;
    position.y = result;

    float tex_coord = (result-minMax[0])/abs(minMax[1]-minMax[0]);
    // color = mix(vec3(0.0, 0.0, 1.0), vec3(1.0, 1.0, 0.0), texCoord);
    // color = vec3((result-minMax[0])/abs(minMax[1]-minMax[0]), 0.0, 1.0);
    color = texture(colormap, tex_coord).rgb;
    gl_Position = projection * view * model * vec4(position, 1.0);
    //gl_Position = vec4(position[0], 1.0);
}

