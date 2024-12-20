#version 330 //!
//? #version 330
#module "engine.common" //!

#export //!
in vec3 v_position;
#export //!
in vec2 v_texCoord;
#export //!
in vec3 v_normal;

#export //!
out vec4 f_color;

#export //!
float getLightStrength()
{
	return clamp((dot(vec3(0.25, 1, 0.5), v_normal) + 1) / 2, 0, 1.0);
}
