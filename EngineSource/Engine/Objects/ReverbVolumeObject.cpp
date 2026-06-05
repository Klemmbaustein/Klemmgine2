#include "ReverbVolumeObject.h"
#include <Engine/Scene.h>
#include <Engine/Engine.h>

void engine::ReverbVolumeObject::Begin()
{
	auto Reload = [this] {
		VolumeData = GetScene()->Sound->AddReverbVolume(this->ObjectTransform, sound::ReverbData{
			.Volume = 1,
			.Density = Density.Value,
			.Gain = Gain.Value,
			.HighFrequencyGain = HighFrequencyGain.Value,
			.LowFrequencyGain = LowFrequencyGain.Value,
			.DecayTime = DecayTime.Value,
			.DecayHighFrequencyRatio = DecayTimeHighFrequencyRatio.Value,
			.DecayLowFrequencyRatio = DecayTimeLowFrequencyRatio.Value,
			.ReflectionsGain = ReflectionsGain.Value,
			.ReflectionsDelay = ReflectionsDelay.Value,
			.ReflectionsPan = ReflectionsPan.Value,
			.LateReverbGain = LateReverbGain.Value,
			.LateReverbDelay = LateReverbDelay.Value,
			.LateReverbPan = LateReverbPan.Value,
			.EchoTime = EchoTime.Value,
			.EchoDepth = EchoDepth.Value,
			.ModulationTime = ModulationTime.Value,
			.ModulationDepth = ModulationDepth.Value,
			.HighFrequencyReference = HighFrequencyReference.Value,
			.LowFrequencyReference = LowFrequencyReference.Value,
			.RoomRolloffFactor = RoomRolloffFactor.Value,
			.DecayHighFrequencyLimit = DecayHighFrequencyLimit.Value
		});
	};

	Reload();

	for (auto& i : this->Properties)
	{
		i->OnChanged = Reload;
	}

	if (!Engine::IsPlaying)
	{
		EditorCollider = new CollisionComponent();
		Attach(EditorCollider);
		EditorCollider->Load(GraphicsModel::UnitCube());
	}
}

void engine::ReverbVolumeObject::Update()
{
	if (EditorCollider)
	{
		EditorCollider->SetCollisionEnabled(GetScene()->Sound->ShowDebugBoundsInEditor);
	}

	if (this->VolumeData->AtTransform != this->ObjectTransform)
	{
		GetScene()->Sound->UpdateReverbVolumeTransform(this->VolumeData, this->ObjectTransform);
	}
}

void engine::ReverbVolumeObject::OnDestroyed()
{
	if (EditorCollider)
	{
		Detach(EditorCollider);
		EditorCollider = nullptr;
	}
	GetScene()->Sound->RemoveReverbVolume(VolumeData);
}
