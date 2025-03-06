//? #version 430

uniform bool u_useTexture = false;
uniform sampler2D u_texture;

in vec2 g_texCoord;

void main()
{
	if (u_useTexture && texture(u_texture, g_texCoord).w < 0.33)
		discard;
}