#version 330
#using "engine.common" //! #include "engine.common.glsl"

#param //!
uniform sampler2D u_texture;
#param //!
uniform bool u_useTexture;
#param //!
uniform vec3 u_color = vec3(1, 1, 1);

void main()
{
	vec3 pixelColor = u_useTexture ? texture(u_texture, v_texCoord).xyz : vec3(1);
	f_color = vec4(getLightStrength() * pixelColor * u_color, 1);
}
