#include "Framebuffer.h"
#include "Engine/Subsystem/VideoSubsystem.h"
#include "Engine/Engine.h"
#include <gl/glew.h>

engine::graphics::Framebuffer::Framebuffer(int64 Width, int64 Height)
{
	Textures[0] = 0;
	Textures[1] = 0;
	Resize(Width, Height);
}
engine::graphics::Framebuffer::~Framebuffer()
{
	glDeleteFramebuffers(1, &Buffer);
	glDeleteTextures(NUM_TEXTURES, Textures);
}

void engine::graphics::Framebuffer::Resize(int64 NewWidth, int64 NewHeight)
{
	if (Width == NewWidth && Height == NewHeight)
	{
		return;
	}

	this->Width = NewWidth;
	this->Height = NewHeight;
	if (Buffer)
	{
		glDeleteFramebuffers(1, &Buffer);
		glDeleteTextures(NUM_TEXTURES, Textures);
	}

	glGenFramebuffers(1, &Buffer);
	glGenTextures(NUM_TEXTURES, Textures);

	glBindTexture(GL_TEXTURE_2D, Textures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0,
		GL_RGBA16F, GLsizei(NewWidth), GLsizei(NewHeight), 0, GL_RGBA,
		GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float Color[4] = { 0, 0, 0, 1 };
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, Color);

	glBindTexture(GL_TEXTURE_2D, Textures[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, GLsizei(NewWidth), GLsizei(NewHeight), 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	Bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Textures[0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, Textures[1], 0);
	Unbind();
}
void engine::graphics::Framebuffer::Bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, Buffer);
}
void engine::graphics::Framebuffer::Unbind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
