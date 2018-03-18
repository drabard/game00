#version 330

out vec4 color;

in vec2 tex;

uniform sampler2D tex0;

void main()
{
	color = vec4(0.0f, 0.0f, 0.0f, texture(tex0, tex).r);
}
