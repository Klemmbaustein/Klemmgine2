#pragma once
#include <Engine/Objects/SceneObject.h>
#include <Engine/Sound/Sound.h>
#include <Engine/Objects/Components/CollisionComponent.h>

namespace engine
{
	class ReverbVolumeObject : public SceneObject
	{
	public:

		ENGINE_OBJECT(ReverbVolumeObject, "Engine/SceneObject", "Engine/Sound");

		void Begin() override;
		void Update() override;
		void OnDestroyed() override;

		PROPERTY(float, Density, =, 1.0f);
		PROPERTY(float, Gain, =, 1.0f);
		PROPERTY(float, HighFrequencyGain, =, 0.9f);
		PROPERTY(float, LowFrequencyGain, =, 0.9f);
		PROPERTY(float, DecayTime, =, 1.5f);
		PROPERTY(float, DecayTimeHighFrequencyRatio, =, 1.0f);
		PROPERTY(float, DecayTimeLowFrequencyRatio, =, 1.0f);
		PROPERTY(float, ReflectionsGain, =, 0.05f);
		PROPERTY(float, ReflectionsDelay, =, 0.007f);
		PROPERTY(Vector3, ReflectionsPan, =, 0);
		PROPERTY(float, LateReverbGain, =, 1.0f);
		PROPERTY(float, LateReverbDelay, =, 0.0f);
		PROPERTY(Vector3, LateReverbPan, =, 0.0f);
		PROPERTY(float, EchoTime, =, 0.25f);
		PROPERTY(float, EchoDepth, =, 0);
		PROPERTY(float, ModulationTime, =, 0.25f);
		PROPERTY(float, ModulationDepth, =, 0);
		PROPERTY(float, HighFrequencyReference, =, 5000.0f);
		PROPERTY(float, LowFrequencyReference, =, 250.0f);
		PROPERTY(float, RoomRolloffFactor, =, 250.0f);
		PROPERTY(int32, DecayHighFrequencyLimit, =, 1);

	private:
		sound::SoundReverbVolume* VolumeData = nullptr;
		CollisionComponent* EditorCollider = nullptr;
	};
}