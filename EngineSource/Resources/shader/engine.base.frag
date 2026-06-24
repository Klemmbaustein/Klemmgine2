//? #version 430
#module "engine.base" //!

// Base fragment shader
#export //!
in vec3 v_position;
#export //!
in vec3 v_screenPosition;
#export //!
in vec2 v_texCoord;
#export //!
in vec3 v_normal;
#export //!
in vec3 v_screenNormal;

#export //!
layout(location = 0) out vec4 f_color;
#export //!
layout(location = 1) out vec4 f_position;
#export //!
layout(location = 2) out vec4 f_normal;

#export //!
float opacity = 1.0;
#export //!
bool affectAO = true;

#export //!
uniform vec3 u_cameraPos = vec3(0);

vec3 fragment();

uniform vec3 u_sceneFogColor = vec3(0.0);
uniform float u_sceneFogRange = 0.0;
uniform float u_sceneFogStart = 0.0;

#export //!
vec3 applyFog(vec3 color)
{
	return u_sceneFogRange > 0
		? mix(color, u_sceneFogColor, pow(clamp((length(v_screenPosition) - u_sceneFogStart) / u_sceneFogRange, 0.0, 1.0), 1.25))
		: color;
}

void main()
{
	f_color.rgb = fragment();
	f_color.a = opacity;
	if (affectAO)
	{
		f_position = vec4(v_screenPosition, 1);
		f_normal = vec4(v_screenNormal != vec3(0) ? normalize(gl_FrontFacing ? v_screenNormal : -v_screenNormal) : vec3(0, 0, 0), 1);
	}
	else
	{
		f_position = vec4(0);
		f_normal = vec4(0);
	}
}