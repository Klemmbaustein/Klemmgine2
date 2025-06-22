//? #version 330 core
#using "engine.common" //! #include "engine.common.frag"

#param //!
uniform sampler2D u_texture;
#param //!
uniform bool u_useTexture = false;
#param //!
uniform vec3 u_color = vec3(1, 1, 1);
#param //!
uniform vec3 u_emissive = vec3(0, 0, 0);

vec3 fragment()
{
	vec4 pixelColor = u_useTexture ? texture(u_texture, v_texCoord) : vec4(1);

	if (pixelColor.w < 0.5)
		discard;

	return applyLighting(pixelColor.xyz * u_color) + u_emissive;
}
