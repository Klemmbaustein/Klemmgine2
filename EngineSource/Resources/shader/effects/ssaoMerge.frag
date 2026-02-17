//! #version 330

uniform sampler2D u_aoTexture;
uniform sampler2D u_mainTexture;
uniform sampler2D u_position;
layout(location = 0) out vec4 f_color;
in vec2 v_texcoords;

float getAo()
{
	float depth = -texture(u_position, v_texcoords).z;

	vec2 texelSize = 1.0 / vec2(textureSize(u_aoTexture, 0));
	int sampled = 0;
	float result = 0;
	for (int x = -1; x < 2; ++x)
	{
		for (int y = -1; y < 2; ++y)
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			vec2 aoValue = texture(u_aoTexture, v_texcoords + offset).xy;
			float difference = aoValue.y - depth;
			if (difference < 1 && difference > -1)
			{
				result += mix(aoValue.x, 1, min(aoValue.y / 100.0, 1.0));
				sampled++;
			}
		}
	}

	if (sampled == 0.0)
	{
		return 1.0;
	}

	return result / sampled;
}

void main()
{
	float aoStrength = getAo();

	f_color = vec4(texture(u_mainTexture, v_texcoords).xyz * vec3(aoStrength), 1.0);
}