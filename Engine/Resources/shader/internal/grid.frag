//? #version 330 core
#using "engine.common" //! #include "../engine.common.frag"

const float LINE_SIZE = 0.01;

vec3 fragment()
{
	float depth = distance(u_cameraPos, v_position);

	float gridSize = 0.1;
	float lineSize = LINE_SIZE;

	opacity = 0.25;

	vec3 color = vec3(0.75);
		
	if (mod(v_position.x + lineSize * 5.0 / 2.0, gridSize * 10.0) < lineSize * 5.0
		|| mod(v_position.z + lineSize * 5.0 / 2.0, gridSize * 10.0) < lineSize * 5.0)
	{
		color = vec3(1);
		opacity = 0.5;
		lineSize *= 5;
	}
	else if (depth > 10)
	{
		discard;
	}
	else
	{
		opacity /= depth / 4 + 1;
	}

	if (depth > 10)
	{
		opacity /= depth - 9;
	}

	if (mod(v_position.x + lineSize / 2.0, gridSize) > lineSize
		&& mod(v_position.z + lineSize / 2.0, gridSize) > lineSize)
	{
		discard;
	}

	return color;
}
