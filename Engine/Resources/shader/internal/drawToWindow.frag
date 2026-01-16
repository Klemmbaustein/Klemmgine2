//! #version 330

layout(location = 0) out vec4 f_color;
in vec2 v_texcoords;
uniform sampler2D u_texture;
uniform sampler2D u_ui;
uniform sampler2D u_alpha;
uniform vec2 u_pos;
uniform vec2 u_size;


uniform bool u_debugShadowMaps = false;
uniform sampler2DArray u_shadowMaps;


void main()
{
	vec2 viewportTexCoords = (v_texcoords - u_pos) / u_size;

	if (viewportTexCoords.x >= 0.0 && viewportTexCoords.y >= 0.0
		&& viewportTexCoords.x <= 1.0 && viewportTexCoords.y <= 1.0)
	{
		//f_color.xyz = clamp(texture(u_texture, viewportTexCoords).xyz, vec3(0.0), vec3(1.0));
		if (u_debugShadowMaps)
		{
			f_color = texture(u_shadowMaps, vec3(viewportTexCoords, 0));
		}
		else
		{
			f_color.xyz = clamp(texture(u_texture, viewportTexCoords).xyz, vec3(0.0), vec3(1.0));
		}
	}
	else
	{
		f_color.xyz = vec3(0.0);
	}
	vec4 uiColor = texture(u_ui, v_texcoords);

	float uiAlpha = texture(u_alpha, v_texcoords).x;
	if (uiAlpha > 0.0)
	{
		f_color.xyz = mix(f_color.xyz, uiColor.xyz / uiAlpha, uiAlpha);
	}
	f_color.w = 1.0;
}
