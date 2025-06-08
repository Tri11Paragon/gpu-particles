#ifdef __cplusplus
#include <string>
const std::string shader_particle_2d_frag = R"("
#version 300 es
precision mediump float;

uniform sampler2D tex;

layout (location = 0) out vec4 FragColor;

void main()
{
    FragColor = texture(tex, gl_PointCoord);
}

")";
#endif