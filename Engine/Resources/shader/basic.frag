#version 330

in vec3 v_position;
in vec2 v_texCoord;
in vec3 v_normal;

uniform sampler2D u_texture;

out vec4 f_color;

void main()
{
	vec3 texColor = texture(u_texture, v_texCoord).xyz;
	float lightStrength = clamp((dot(vec3(0.25, 1, 0.5), v_normal) + 1) / 2, 0, 1);
	f_color = vec4(texColor * lightStrength, 1);
}