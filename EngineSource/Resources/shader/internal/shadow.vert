//? #version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 a_uv;

#if ENGINE_INSTANCED
layout(location = 3) in mat4 a_transform;
#endif

out vec2 v_texCoord;

uniform mat4 u_model;

void main()
{
	v_texCoord = a_uv;
#if ENGINE_INSTANCED
	gl_Position = u_model * a_transform * vec4(aPos, 1.0);
#else
	gl_Position = u_model * vec4(aPos, 1.0);
#endif
}