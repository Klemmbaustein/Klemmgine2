#include "AmbientOcclusion.h"
#include "Bloom.h"
#include "FXAA.h"
#include "PostProcess.h"
#include "PostProcessEffect.h"
#include <algorithm>
#include <Engine/Graphics/VideoSubsystem.h>

using namespace engine::graphics;

engine::graphics::PostProcess::PostProcess()
{
	PostProcessBuffers[0] = nullptr;
	PostProcessBuffers[1] = nullptr;
}

engine::graphics::PostProcess::~PostProcess()
{
	for (PostProcessEffect* Effect : this->ActiveEffects)
	{
		delete Effect;
	}

	for (auto& i : PostProcessBuffers)
	{
		delete i;
	}
}

void engine::graphics::PostProcess::Init(uint32 Width, uint32 Height)
{
	this->Width = Width;
	this->Height = Height;
	GenerateBuffers();

	AddEffect(new Bloom());
	AddEffect(new AmbientOcclusion());
	AddEffect(new FXAA());
}

void engine::graphics::PostProcess::OnBufferResized(uint32 Width, uint32 Height)
{
	this->Width = Width;
	this->Height = Height;

	for (auto& i : PostProcessBuffers)
	{
		delete i;
	}

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

RendererTexture* engine::graphics::PostProcess::Draw(Framebuffer* Buffer, Camera* Cam)
{
	RendererTexture* CurrentBuffer = Buffer->Buffer->GetTexture(0);

	for (PostProcessEffect* Effect : this->ActiveEffects)
	{
		CurrentBuffer = Effect->Draw(CurrentBuffer, this, Buffer, Cam);
	}

	return CurrentBuffer;
}

RendererDrawTarget* engine::graphics::PostProcess::NextBuffer()
{
	IsFirstBuffer = !IsFirstBuffer;
	return PostProcessBuffers[IsFirstBuffer];
}

void engine::graphics::PostProcess::SortEffectsByPriority()
{
	std::sort(this->ActiveEffects.begin(), this->ActiveEffects.end(),
		[](graphics::PostProcessEffect* a, graphics::PostProcessEffect* b) {
		return a->DrawOrder < b->DrawOrder;
	});
}

void engine::graphics::PostProcess::GenerateBuffers()
{
	for (auto& i : this->PostProcessBuffers)
	{
		i = VideoSubsystem::Current->Renderer->CreateDrawTarget(Width, Height, {
			DrawTargetBuffer{} });
	}
}
