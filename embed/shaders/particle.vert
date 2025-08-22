#version 450 core
precision mediump float;

layout (location = 0) in vec2 vertex_pos;

layout (std140, binding = 0) buffer particle_positions {
    vec2 pos[];
};

out float silly;

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
    if (mod(position.x + position.y, 32.0f) >= 16.0f)
        silly = 1.0f;
    else
        silly = 0.0f;
    gl_Position = ovm * vec4(position.x, position.y, 0.0, 1.0);
}