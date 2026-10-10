//? #version 330
#module "engine.common" //!

#export //!
layout(location = 0) in vec3 a_position;
#export //!
layout(location = 1) in vec2 a_uv;
#export //!
layout(location = 2) in vec3 a_normal;

#if ENGINE_INSTANCED
layout(location = 3) in mat4 a_transform;
#endif

out vec3 v_position;
out vec3 v_screenPosition;
out vec2 v_texCoord;
out vec3 v_normal;
out vec3 v_screenNormal;

#export //!
uniform mat4 u_model;
#export //!
uniform mat4 u_view;
#export //!
uniform mat4 u_projection;

#export //!
vec3 translatePosition(vec3 inPosition)
{
#if ENGINE_INSTANCED
	return (u_model * a_transform * vec4(inPosition, 1)).xyz;
#else
	return (u_model * vec4(inPosition, 1)).xyz;
#endif
}

vec3 vertex();

void main()
{
	v_position = vertex();
#if ENGINE_INSTANCED
	v_normal = normalize(mat3(u_model) * mat3(a_transform) * a_normal);
#else
	v_normal = normalize(mat3(u_model) * a_normal);
#endif
	v_screenNormal = normalize(mat3(u_view) * v_normal);
	v_texCoord = a_uv;
	v_screenPosition = (u_view * vec4(v_position, 1)).xyz;
	gl_Position = u_projection * vec4(v_screenPosition, 1);
}
