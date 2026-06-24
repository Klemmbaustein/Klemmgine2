//? #version 330 core
#using "engine.base" //! #include "engine.base.frag"
#unlit

#param
uniform sampler2D u_sprite;
#param
uniform vec3 u_color = vec3(1);

vec3 fragment()
{
	vec4 textureValue = texture(u_sprite, v_texCoord);

	if (textureValue.a < 0.5)
	{
		discard;
	}

	opacity = textureValue.a;
	return applyFog(textureValue.xyz / textureValue.a * u_color);
}