#include "PostProcessEffect.h"
#include <Engine/Internal/OpenGL.h>
#include <Core/Log.h>

engine::graphics::PostProcessEffect::PostProcessEffect()
{
}

engine::graphics::PostProcessEffect::~PostProcessEffect()
{
}

void engine::graphics::PostProcessEffect::OnBufferResized(uint32 Width, uint32 Height)
{
}


uint64 engine::graphics::PostProcessEffect::CreateNewBuffer(uint32 Width, uint32 Height, bool FilterNearest)
{
	uint32 Buffer, Texture;
	glGenFramebuffers(1, &Buffer);
	glGenTextures(1, &Texture);
	glBindFramebuffer(GL_FRAMEBUFFER, Buffer);
	glBindTexture(GL_TEXTURE_2D, Texture);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA16F,
		Width,
		Height,
		0,
		GL_RGBA,
		GL_FLOAT,
		NULL
	);

	GLint Filtering = FilterNearest ? GL_NEAREST : GL_LINEAR;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Filtering);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Filtering);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Texture, 0
	);

	return uint64(Buffer) | uint64(Texture) << 32;
}

uint32 engine::graphics::PostProcessEffect::DrawBuffer(uint64 Buffer, uint32 Width, uint32 Height)
{
	uint32 FrameBuffer = uint32(Buffer), Texture = uint32(Buffer >> 32);

	glViewport(0, 0, Width, Height);

	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	return Texture;
}

void engine::graphics::PostProcessEffect::FreeBuffer(uint64 Buffer)
{
}

