#include "SoundComponent.h"
#include <Engine/Scene.h>
#include <Engine/Engine.h>

engine::SoundComponent::SoundComponent()
{
}

engine::SoundComponent::~SoundComponent()
{
	ClearBuffer();
}

void engine::SoundComponent::Load(AssetRef Sound)
{
	ClearBuffer();

	if (!Sound.IsValid())
	{
		return;
	}

	if (!RootObject)
	{
		Log::Warn(str::Format("Tried to play sound %s but the component is not attached to anything so there is no scene to play sound in.", Sound.FilePath.c_str()));
		return;
	}

	Context = RootObject->GetScene()->Sound;

	if (!Context)
	{
		return;
	}

	Buffer = Context->LoadSoundEffect(Sound.FilePath);
	if (Buffer)
	{
		Source = Context->CreateSoundSource(Buffer);
	}
	else
	{
		Log::Warn("Failed to load sound effect: " + Sound.FilePath);
	}
}

void engine::SoundComponent::Play(bool Loop, bool Is3D)
{
	if (Context && Source && Engine::IsPlaying)
	{
		Update();
		Context->PlaySource(Source, Loop, Is3D);
	}
}

void engine::SoundComponent::Stop()
{
	if (Context && Source)
	{
		Context->StopSource(Source);
	}
}

void engine::SoundComponent::SetRange(float Range)
{
	if (Context && Source)
	{
		Context->SetSourceRange(Source, Range);
	}
}

void engine::SoundComponent::SetVolume(float Volume)
{
	if (Context && Source)
	{
		Context->SetSourceVolume(Source, Volume);
	}
}

void engine::SoundComponent::SetPitch(float Pitch)
{
	if (Context && Source)
	{
		Context->SetSourcePitch(Source, Pitch);
	}
}

void engine::SoundComponent::Update()
{
	if (Source && Context)
	{
		Context->SetSourcePosition(Source, WorldTransform.ApplyTo(0));
	}
}

void engine::SoundComponent::ClearBuffer()
{
	if (!Context)
	{
		return;
	}

	if (Source)
	{
		Context->FreeSoundSource(Source);
		Source = nullptr;
	}
	if (Buffer)
	{
		Context->FreeSoundEffect(Buffer);
		Buffer = nullptr;
	}
}
