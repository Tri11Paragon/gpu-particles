#version 430
precision mediump float;

uniform sampler2D tex1;
uniform sampler2D tex2;

in vec2 uv;
in float silly;

layout (location = 0) out vec4 FragColor;

void main()
{
    if (silly > 0.0f)
        FragColor = texture(tex1, uv);
    else
        FragColor = texture(tex2, uv);
}