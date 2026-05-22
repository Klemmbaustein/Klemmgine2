#include "ReverbVolumeObject.h"
#include <Engine/Scene.h>
#include <Engine/Engine.h>

void engine::ReverbVolumeObject::Begin()
{
	Volume = GetScene()->Sound->AddReverbVolume(this->ObjectTransform);
	if (!Engine::IsPlaying)
	{
		GetScene()->Sound->Debug_ShowReverbAreas(&GetScene()->Graphics.Debug);
	}
}

void engine::ReverbVolumeObject::Update()
{
	if (this->Volume->AtTransform != this->ObjectTransform)
	{
		GetScene()->Sound->UpdateReverbVolumeTransform(this->Volume, this->ObjectTransform);
		if (!Engine::IsPlaying)
		{
			GetScene()->Sound->Debug_ShowReverbAreas(&GetScene()->Graphics.Debug);
		}
	}
}

void engine::ReverbVolumeObject::OnDestroyed()
{
	GetScene()->Sound->RemoveReverbVolume(Volume);
	if (!Engine::IsPlaying)
	{
		GetScene()->Sound->Debug_ShowReverbAreas(&GetScene()->Graphics.Debug);
	}
}
