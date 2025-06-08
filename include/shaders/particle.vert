#ifdef __cplusplus
#include <string>
const std::string shader_particle_2d_vert = R"("
#version 300 es
precision mediump float;

layout (location = 0) in vec2 position;

layout (std140) uniform GlobalMatrices
{
    mat4 projection;
    mat4 ortho;
    mat4 view;
    mat4 pvm;
    mat4 ovm;
};

void main()
{
    gl_Position = ovm * vec4(position.x, position.y, 0.0, 1.0);
}

")";
#endif