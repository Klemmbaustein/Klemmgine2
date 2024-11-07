#version 330

in vec2 v_texcoords;
in vec2 v_position;
in float v_cornerIndex;
layout (location = 0) out vec4 f_color;
layout (location = 1) out vec4 f_alpha;

uniform vec3 u_color = vec3(1);
uniform vec3 u_borderColor = vec3(1);
uniform sampler2D u_texture;
uniform vec3 u_offset; // Scroll bar: X = scrolled distance; Y = MaxDistance; Z MinDistance
uniform float u_opacity = 1;
uniform bool u_drawBorder = false;
uniform bool u_drawCorner = true;
uniform float u_borderScale = 0;
uniform float u_cornerScale = 0;
uniform vec4 u_transform = vec4(vec2(0), vec2(1));
uniform float u_aspectRatio = 16.0/9.0;
uniform vec2 u_screenRes = vec2(1600, 900);
uniform int u_cornerFlags = 0;
uniform int u_borderFlags = 0;

#define NUM_SAMPLES 2

bool isBorderVisible(int index)
{
	return (u_borderFlags & (1 << index)) != 0;
}

float rand(vec2 n)
{ 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

void main()
{
	vec2 scale = u_transform.zw * vec2(u_aspectRatio, 1);
	vec2 scaledTexCoords = v_texcoords * scale * (1 + 0.5 / u_screenRes);
	vec2 nonAbsCenteredTexCoords = (scaledTexCoords - scale / 2) * 2;
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

	f_color = vec4(u_color, 1);
	if (u_drawCorner && (u_cornerFlags & (1 << cornerIndex)) != 0
		&& (centeredTexCoords.y >= scale.y - u_cornerScale) && (centeredTexCoords.x >= scale.x - u_cornerScale))
	{
		float borderSize = pow((length((scale - u_cornerScale) - centeredTexCoords) / u_cornerScale), u_cornerScale * 1000);
		f_color.a *= clamp(1.0 / borderSize, 0, 1);

		if (u_drawBorder && u_cornerScale > u_borderScale)
		{
			float cornerDistance = (length((scale - u_cornerScale) - centeredTexCoords));
			f_color.rgb = mix(f_color.rgb, u_borderColor, clamp((u_borderScale - (u_cornerScale - cornerDistance)) / u_borderScale * 4, 0, 1));
		}
	}
	f_alpha.xyz = vec3(0);
	f_alpha.w = f_color.w;
}