//? #version 330
#using "engine.common" //! #include "engine.common.frag"

#param //!
uniform sampler2D u_texture;
#param //!
uniform bool u_useTexture = false;
#param //!
uniform vec3 u_color = vec3(1, 1, 1);

void main()
{
	vec4 pixelColor = u_useTexture ? texture(u_texture, v_texCoord) : vec4(1);

	if (pixelColor.w < 0.5)
		discard;

	f_color = vec4(getLightStrength() * pixelColor.xyz * u_color, 1);
}
