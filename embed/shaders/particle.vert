#version 460 core

layout (location = 0) in vec2 vertex_pos;
layout (location = 1) in vec2 uv_in;
layout (location = 2) in uint alive_index;

out vec2 uv;
out float silly;

layout (std430, binding = 1) buffer particle_positions {
    vec2 pos[];
};

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
    uv = uv_in;
    vec2 position = vertex_pos + pos[alive_index];
    if (mod(position.x + position.y, 32.0f) >= 16.0f)
        silly = 1.0f;
    else
        silly = 0.0f;
    gl_Position = ovm * vec4(position.x, position.y, 0.0, 1.0);
}