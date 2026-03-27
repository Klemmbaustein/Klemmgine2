#include "SoundComponent.h"
#include <Engine/Scene.h>

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
		return;
	}

	Context = RootObject->GetScene()->Sound;

	if (!Context)
	{
		return;
	}

	Buffer = Context->LoadSoundEffect(Sound.FilePath);
	Source = Context->CreateSoundSource(Buffer);
}

void engine::SoundComponent::Play()
{
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
