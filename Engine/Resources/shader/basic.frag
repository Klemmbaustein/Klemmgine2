#version 330
#using "engine.common"

void main()
{
	f_color = vec4(vec3(getLightStrength()), 1);
}
