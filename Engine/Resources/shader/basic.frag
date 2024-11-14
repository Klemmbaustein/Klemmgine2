#version 330

in vec3 v_position;
in vec3 v_normal;

out vec4 f_color;

void main()
{
	float lightStrength = clamp(dot(vec3(0.25, 1, 0.5), v_normal), 0, 1);
	f_color = vec4(vec3(1, 0, 0) * lightStrength, 1) + vec4(0, 0, 0.1, 0);
}