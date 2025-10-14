//? #version 330 core
#using "engine.common" //! #include "../engine.common.frag"

const float LINE_SIZE = 0.01;

vec3 fragment()
{
	if (mod(v_position.x + LINE_SIZE / 2.0, 0.1) > LINE_SIZE
		&& mod(v_position.z + LINE_SIZE / 2.0, 0.1) > LINE_SIZE)
	{
		discard;
	}

	return vec3(1);
}
