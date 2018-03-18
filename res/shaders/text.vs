#version 330

layout(location = 0) in vec3 pos;
layout(location = 2) in vec3 texcoord;

uniform vec2 position;

out vec2 tex;
void main()
{
    gl_Position = vec4(pos.xy + position, 0.0f, 1.0f);
    tex = texcoord.xy;
}
