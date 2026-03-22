//! #version 330
in vec2 v_texcoords;
in vec2 v_position;
in float v_cornerIndex;
layout (location = 0) out vec4 f_color;
layout (location = 1) out vec4 f_alpha;

uniform vec3 u_color;
uniform vec4 u_scrollBounds;
uniform float u_opacity;

void main()
{
	if (u_scrollBounds.x > v_position.x)
	{
		discard;
	}
	if (u_scrollBounds.y < v_position.x)
	{
		discard;
	}

	if (u_scrollBounds.z > v_position.y)
	{
		discard;
	}
	if (u_scrollBounds.w < v_position.y)
	{
		discard;
	}

	f_color = vec4(u_color, u_opacity);
	f_alpha.xyz = vec3(0);
	f_alpha.w = f_color.w;
	f_color.xyz = vec3(0);
	f_color.w = 1.0;
}