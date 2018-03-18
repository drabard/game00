#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 texcoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 pos;
out vec3 norm;
out vec2 tex;
void main()
{
  pos = position;
  norm = normal;
  tex = texcoord.xy;
  gl_Position = projection*view*model*vec4(position.xyz, 1.0f);
}
