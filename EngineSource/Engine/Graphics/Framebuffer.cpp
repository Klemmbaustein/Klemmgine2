#include "Framebuffer.h"
#include "Engine/Subsystem/VideoSubsystem.h"
#include "Engine/Engine.h"
#include <gl/glew.h>

engine::graphics::Framebuffer::Framebuffer(int64 Width, int64 Height)
{
	this->Width = Width;
	this->Height = Height;
	Resize(Width, Height);
}
engine::graphics::Framebuffer::~Framebuffer()
{
	glDeleteFramebuffers(1, &Buffer);
	glDeleteTextures(1, &Texture);
}

void engine::graphics::Framebuffer::Resize(int64 NewWidth, int64 NewHeight)
{
	this->Width = NewWidth;
	this->Height = NewHeight;
	if (Buffer)
	{
		glDeleteFramebuffers(1, &Buffer);
		glDeleteTextures(1, &Texture);
	}

	glGenFramebuffers(1, &Buffer);
	glGenTextures(1, &Texture);

	glBindTexture(GL_TEXTURE_2D, Texture);
	glTexImage2D(GL_TEXTURE_2D, 0,
		GL_RGBA16F, GLsizei(NewWidth), GLsizei(NewHeight), 0, GL_RGBA,
		GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float Color[4] = { 1, 0, 1, 1 };
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, Color);

	Bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Texture, 0);
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
