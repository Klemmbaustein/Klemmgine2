#include "PostProcess.h"
#include "PostProcessEffect.h"
#include <Engine/Internal/OpenGL.h>
#include <algorithm>
#include "Bloom.h"
#include "AmbientOcclusion.h"

engine::graphics::PostProcess::PostProcess()
{
	PostProcessBuffers[0] = 0;
	PostProcessBuffers[1] = 0;
	PostProcessTextures[0] = 0;
	PostProcessTextures[1] = 0;
}

engine::graphics::PostProcess::~PostProcess()
{
	for (PostProcessEffect* Effect : this->ActiveEffects)
	{
		delete Effect;
	}

	glDeleteFramebuffers(2, PostProcessBuffers.data());
	glDeleteTextures(2, PostProcessTextures.data());
}

void engine::graphics::PostProcess::Init(uint32 Width, uint32 Height)
{
	this->Width = Width;
	this->Height = Height;
	GenerateBuffers();

	AddEffect(new Bloom());
	AddEffect(new AmbientOcclusion());
}

void engine::graphics::PostProcess::OnBufferResized(uint32 Width, uint32 Height)
{
	this->Width = Width;
	this->Height = Height;
	glDeleteFramebuffers(2, PostProcessBuffers.data());
	glDeleteTextures(2, PostProcessTextures.data());
	GenerateBuffers();

	for (PostProcessEffect* Effect : this->ActiveEffects)
	{
		Effect->OnBufferResized(Width, Height);
	}
}

void engine::graphics::PostProcess::AddEffect(PostProcessEffect* NewEffect)
{
	ActiveEffects.push_back(NewEffect);
	NewEffect->OnBufferResized(this->Width, this->Height);

	SortEffectsByPriority();
}

void engine::graphics::PostProcess::RemoveEffect(PostProcessEffect* TargetEffect)
{
	for (auto i = ActiveEffects.begin(); i < ActiveEffects.end(); i++)
	{
		if (*i == TargetEffect)
		{
			ActiveEffects.erase(i);
			break;
		}
	}
}

uint32 engine::graphics::PostProcess::Draw(Framebuffer* Buffer, Camera* Cam)
{
	uint32 CurrentBuffer = Buffer->Textures[0];

	for (PostProcessEffect* Effect : this->ActiveEffects)
	{
		CurrentBuffer = Effect->Draw(CurrentBuffer, this, Buffer, Cam);
	}

	return CurrentBuffer;
}

std::pair<uint32, uint32> engine::graphics::PostProcess::NextBuffer()
{
	uint32 Buffer = PostProcessBuffers[IsFirstBuffer],
		Texture = PostProcessTextures[IsFirstBuffer];

	IsFirstBuffer = !IsFirstBuffer;
	return std::pair{ Buffer, Texture };
}

void engine::graphics::PostProcess::SortEffectsByPriority()
{
	std::sort(this->ActiveEffects.begin(), this->ActiveEffects.end(),
		[](graphics::PostProcessEffect* a, graphics::PostProcessEffect* b)
		{
			return a->DrawOrder < b->DrawOrder;
		});
}

void engine::graphics::PostProcess::GenerateBuffers()
{
	glGenFramebuffers(2, PostProcessBuffers.data());
	glGenTextures(2, PostProcessTextures.data());

	for (size_t i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, PostProcessBuffers[i]);
		glBindTexture(GL_TEXTURE_2D, PostProcessTextures[i]);
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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, PostProcessTextures[i], 0
		);
	}
}
