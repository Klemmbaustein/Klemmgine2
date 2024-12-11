#version 330

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec3 a_normal;

out vec3 v_position;
out vec2 v_texCoord;
out vec3 v_normal;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
	vec4 worldPos = u_model * vec4(a_position, 1);
	v_position = worldPos.xyz;
	v_normal = a_normal;
	v_texCoord = a_uv;
	gl_Position = u_projection * u_view * worldPos;
}