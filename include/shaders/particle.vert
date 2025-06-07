#ifdef __cplusplus
#include <string>
const std::string shader_particle_2d_vert = R"("
#version 300 es
precision mediump float;

layout (location = 0) in vec4 position_uv;

out vec2 UVs;

void main()
{
    gl_Position = vec4(position_uv.x, position_uv.y, 0.0, 1.0);
    UVs = position_uv.zw;
}

")";
#endif