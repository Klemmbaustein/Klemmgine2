#version 330

layout(location = 0) out vec4 f_color;
in vec2 v_texcoords;
uniform sampler2D u_texture;
uniform sampler2D u_ui;
uniform sampler2D u_alpha;
uniform vec2 u_pos;
uniform vec2 u_size;
uniform sampler2DArray u_shadowMap;

void main()
{
	vec2 viewportTexCoords = (v_texcoords - u_pos) / u_size;
	f_color.xyz = texture(u_texture, viewportTexCoords).xyz;
	vec4 uiColor = texture(u_ui, v_texcoords);

	float uiAlpha = texture(u_alpha, v_texcoords).x;
	if (uiAlpha > 0)
		f_color.xyz = mix(f_color.xyz, uiColor.xyz / uiAlpha, uiAlpha);
	f_color.w = 1;
}