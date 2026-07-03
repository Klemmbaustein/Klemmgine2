#using "engine.base" //! #include "../engine.base.frag"
#unlit //!

#param //!
uniform vec3 u_color = vec3(1, 0, 0);

vec3 fragment()
{
	vec2 coords = vec2((v_texCoord.x - 0.5) * 2, (v_texCoord.y - 0.5) * 2);

	coords = max(pow(coords, vec2(2.0)) - vec2(0.5), 0.1);

	opacity = max(abs(coords.x), abs(coords.y));
	affectAO = false;
	return applyFog(u_color);
}