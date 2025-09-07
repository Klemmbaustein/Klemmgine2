//! #version 330
#using "engine.common" //! #include "engine.common.frag"

#param //!
uniform vec3 u_skycolor = vec3(0.3, 0.6, 1);
#param //!
uniform vec3 u_horizoncolor = vec3(0.7, 0.8, 1);

vec3 fragment()
{
	vec3 normal = normalize(v_normal);
	vec3 color = mix(u_horizoncolor, u_skycolor, abs(dot(vec3(normal), vec3(0, 1, 0))));

	float dir = max(pow(dot(-normal, normalize(u_lightDirection)), 5000.0), 0.0);

	return mix(color, vec3(2, 1.7, 0.5) * 3.0, dir);
}