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
layout(location = 1) out vec3 f_position;
#export //!
layout(location = 2) out vec3 f_normal;

#export //!
float opacity = 1.0;

#export //!
uniform vec3 u_cameraPos = vec3(0);

vec3 fragment();

void main()
{
	f_color.rgb = fragment();
	f_color.a = opacity;
	f_position = v_screenPosition;
	f_normal = v_screenNormal != vec3(0) ? normalize(v_screenNormal) : vec3(0, 0, 0);
}