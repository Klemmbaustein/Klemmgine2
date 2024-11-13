#version 330

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;

out vec3 v_position;
out vec3 v_normal;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
	v_position = a_position;
	v_normal = a_normal;
	gl_Position = u_projection * u_view * u_model * vec4(v_position, 1);
}