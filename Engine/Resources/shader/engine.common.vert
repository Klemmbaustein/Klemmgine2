//? #version 330
#module "engine.common" //!

#export //!
layout(location = 0) in vec3 a_position;
#export //!
layout(location = 1) in vec2 a_uv;
#export //!
layout(location = 2) in vec3 a_normal;

out vec3 v_position;
out vec3 v_screenPosition;
out vec2 v_texCoord;
out vec3 v_normal;

#export //!
uniform mat4 u_model;
#export //!
uniform mat4 u_view;
#export //!
uniform mat4 u_projection;

#export //!
vec3 WorldPosToScreenPos(vec3 inPosition)
{
	return (u_model * vec4(inPosition, 1)).xyz;
}

#export //!
void SetScreenPosition(vec3 inScreenPos)
{
	v_position = inScreenPos;
	v_normal = normalize(mat3(u_model) * a_normal);
	v_texCoord = a_uv;
	v_screenPosition = (u_view * vec4(inScreenPos, 1)).xyz;
	gl_Position = u_projection * vec4(v_screenPosition, 1);
}
