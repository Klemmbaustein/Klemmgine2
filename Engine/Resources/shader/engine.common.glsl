#version 330
#module "engine.common"

// Pixel position
#export
in vec3 v_position;

// Pixel texture coordinate
#export
in vec2 v_texCoord;

// Pixel normal
#export
in vec3 v_normal;

// Final pixel color
#export
out vec4 f_color;

#export
float getLightStrength()
{
	return clamp((dot(vec3(0.25, 1, 0.5), v_normal) + 1) / 2, 0, 1.0);
}
