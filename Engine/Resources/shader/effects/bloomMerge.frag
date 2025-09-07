//! #version 330

uniform sampler2D u_bloomTexture;
uniform sampler2D u_mainTexture;
layout(location = 0) out vec4 f_color;
in vec2 v_texcoords;

void main()
{
	vec3 bloomColor = texture(u_bloomTexture, v_texcoords).rgb;
	float bloomStrength = max(length(bloomColor) - 0.75, 0);

	f_color = vec4(
		mix(
			texture(u_mainTexture, v_texcoords).rgb,
			bloomColor + bloomStrength / 40,
			min(0.02 + bloomStrength * 0.2, 1)),
	1);
}