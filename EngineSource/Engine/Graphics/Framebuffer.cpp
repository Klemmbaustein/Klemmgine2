#include "Framebuffer.h"
#include <Engine/Graphics/VideoSubsystem.h>

engine::graphics::Framebuffer::Framebuffer(int64 Width, int64 Height)
{
	Resize(Width, Height);
}
engine::graphics::Framebuffer::~Framebuffer()
{
	delete this->Buffer;
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
		delete Buffer;
	}

	Buffer = VideoSubsystem::Current->Renderer->CreateDrawTarget(Width, Height, {
		DrawTargetBuffer{
			.Type = DrawTargetBufferType::Color,
		},
		DrawTargetBuffer{
			.Type = DrawTargetBufferType::DepthStencil,
		},
		DrawTargetBuffer{
			.Type = DrawTargetBufferType::Color,
		},
		DrawTargetBuffer{
			.Type = DrawTargetBufferType::Color,
		}, });
}

void engine::graphics::Framebuffer::Bind() const
{
	this->Buffer->Activate();
}
