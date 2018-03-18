#version 330

out vec4 color;

in vec3 pos;
in vec3 norm;
in vec2 tex;

uniform vec3 light_pos;
uniform sampler2D tex0;
uniform mat4 model;

void main()
{
	vec4 light_dir = inverse(model)*vec4(light_pos - pos, 1.0f);
	float light_intensity = 0.8f;
	float light_mul = light_intensity * dot(norm, normalize(vec3(light_dir)));
	float ambient_intensity = 0.2f;

	vec3 diffuse_color = texture(tex0, tex).xyz;
	color = vec4(diffuse_color * (light_mul + ambient_intensity), 1.0f);
}