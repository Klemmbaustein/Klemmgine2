#version 330

layout(location = 0) out vec4 f_color;
in vec2 v_texcoords;
uniform sampler2D u_ui;

void main()
{
	f_color.xyz = texture(u_ui, v_texcoords).xyz;
	f_color.w = 1;
}