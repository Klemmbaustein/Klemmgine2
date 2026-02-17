//! #version 330
in vec2 v_texcoords;
in vec2 v_position;
in float v_cornerIndex;
layout (location = 0) out vec4 f_color;
layout (location = 1) out vec4 f_alpha;

uniform vec3 u_color;
uniform sampler2D u_texture;
uniform vec3 u_offset; // Scroll bar: X = scrolled distance; Y = MaxDistance; Z MinDistance
uniform float u_opacity;
uniform vec4 u_transform;
uniform float u_aspectRatio;
uniform vec2 u_screenRes;

void main()
{
	vec2 scale = u_transform.zw * vec2(u_aspectRatio, 1.0);
	vec2 scaledTexCoords = v_texcoords * scale * (1.0 + 0.5 / u_screenRes);
	vec2 nonAbsCenteredTexCoords = (scaledTexCoords - scale / 2.0) * 2.0;
	vec2 centeredTexCoords = abs(nonAbsCenteredTexCoords);

	int cornerIndex = int(round(v_cornerIndex));

	if (u_offset.y > v_position.y)
	{
		discard;
	}
	if (u_offset.z < v_position.y)
	{
		discard;
	}

	f_color = vec4(u_color, u_opacity);
	f_alpha.xyz = vec3(0);
	f_alpha.w = f_color.w;
	f_color.xyz = vec3(0);
	f_color.w = 1.0;
}