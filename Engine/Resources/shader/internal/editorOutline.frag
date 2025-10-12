//! #version 330

layout(location = 0) out vec4 f_color;
in vec2 v_texcoords;
uniform sampler2D u_texture;
uniform usampler2D u_depthStencil;
uniform bool u_isHovered = false;

void main()
{
	vec3 baseColor = texture(u_texture, v_texcoords).xyz;

	vec2 texelSize = 2.0 / textureSize(u_depthStencil, 0);
	vec2 texCoord = v_texcoords - texelSize / 2;

	uint baseValue = texture(u_depthStencil, texCoord).x;

	uint stencilValue1 = texture(u_depthStencil, texCoord + vec2(0, 1) * texelSize).x;
	uint stencilValue2 = texture(u_depthStencil, texCoord + vec2(1, 0) * texelSize).x;
	uint stencilValue3 = texture(u_depthStencil, texCoord + vec2(1, 1) * texelSize).x;

	if (baseValue > 1u
		|| stencilValue1 > 1u
		|| stencilValue2 > 1u
		|| stencilValue3 > 1u)
	{
		uint highest = max(max(baseValue, stencilValue1), max(stencilValue2, stencilValue3));

		switch (highest)
		{
		case 2u:
			f_color.xyz = vec3(1, 0, 0) + (u_isHovered ? vec3(0.5) : vec3(0));
			break;
		case 3u:
			f_color.xyz = vec3(0, 1, 0) + (u_isHovered ? vec3(0.5) : vec3(0));
			break;
		case 4u:
			f_color.xyz = vec3(0, 0, 1) + (u_isHovered ? vec3(0.5) : vec3(0));
			break;
		}
	}
	else if (baseValue != stencilValue1
		|| baseValue != stencilValue2
		|| baseValue != stencilValue3)
	{
		f_color.xyz = vec3(1);
	}
	else
	{
		f_color.xyz = baseColor;
	}
	f_color.w = 1;
}