//? #version 330
#using "engine.common" //! #include "engine.common.frag"

vec3 vertex()
{
	return translatePosition(a_position);
}
