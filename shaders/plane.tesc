#version 410 core

layout (vertices=4) out;

// input from vertex shader
in vec3 color[];
in vec3 position[];

// output to evaluation shader
out vec3 outColor[];
out vec3 outPosition[];

// uniform mat4 model;
// uniform mat4 view;
// uniform mat4 projection;

// same for gl_out
// in gl_PerVertex
// {
//     vec4 gl_Position;
//     float gl_PointSize;
//     float gl_ClipDistance[];
// } gl_in[gl_MaxPatchVertices];

// gl_InvocationID - currently processed vertex of the patch

void main() {
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    outPosition[gl_InvocationID] = position[gl_InvocationID];
    outColor[gl_InvocationID] = color[gl_InvocationID];

    // invocation 0 controls tesselation levels for the whole patch
    if (gl_InvocationID == 0) {
        // for each edge of the quad, number of subdivisions
        gl_TessLevelOuter[0] = 5;
        gl_TessLevelOuter[1] = 5;
        gl_TessLevelOuter[2] = 5;
        gl_TessLevelOuter[3] = 5;

        gl_TessLevelInner[0] = 5;
        gl_TessLevelInner[1] = 5;
    }
}
