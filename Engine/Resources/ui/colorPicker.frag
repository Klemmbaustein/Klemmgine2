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

uniform int u_mode = 1;
uniform float u_selectedHue;
uniform vec2 selectedPos = vec2(0);

bool NearlyEqual(float A, float B, float epsilon)
{
	return (abs(A - B) < epsilon);
}

vec3 rgb2hsv(vec3 c)
{
	vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
	vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
	vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

	float d = q.x - min(q.w, q.y);
	float e = 1.0e-10;
	return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

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

	if (u_mode == 0)
	{
		f_color.xyz = hsv2rgb(vec3(u_selectedHue, v_texcoords.x, v_texcoords.y));
		float dst = length(selectedPos.xy - v_texcoords);
		if (dst < 0.03 && dst > 0.02)
		{
			f_color = vec4(1);
		}
	}
	else
	{
		vec3 hueVal = hsv2rgb(vec3(v_texcoords.y, 1, 1));

		if (NearlyEqual(u_selectedHue, v_texcoords.y, 0.01))
		{
			f_color = vec4(1);
		}
		else
		{
			f_color = vec4(hueVal, 1);
		}
	}
	f_alpha.xyz = vec3(1);
	f_alpha.w = 1;
	f_color.w = 1.0;
}