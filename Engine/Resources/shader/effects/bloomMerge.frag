//! #version 330

uniform sampler2D u_bloomTexture;
uniform sampler2D u_mainTexture;
layout(location = 0) out vec4 f_color;
in vec2 v_texcoords;

void main()
{
	f_color = vec4(
		mix(texture(u_mainTexture, v_texcoords).rgb, texture(u_bloomTexture, v_texcoords).rgb, 0.15),
	1);
}