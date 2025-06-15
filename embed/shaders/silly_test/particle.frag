#version 300 es
precision mediump float;

uniform sampler2D tex1;
uniform sampler2D tex2;

in float silly;

layout (location = 0) out vec4 FragColor;

void main()
{
    float x = gl_PointCoord.x;
    float y = 1.0f - gl_PointCoord.y;
    if (silly > 0.0f)
        FragColor = texture(tex1, vec2(x, y));
    else
        FragColor = texture(tex2, vec2(x, y));
}