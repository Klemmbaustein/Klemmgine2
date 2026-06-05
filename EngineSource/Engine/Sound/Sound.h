#pragma once
#include <Engine/Subsystem/Subsystem.h>
#include "Sources/SoundFileSource.h"
#include <Core/Vector.h>
#include <Engine/Graphics/Camera.h>
#include <Engine/Graphics/Scene/BoundsHirarchy.h>
#include <Engine/Debug/DebugDraw.h>
#include <memory>
#include <mutex>

namespace engine::sound
{
	struct SoundDevice_Private;
	struct SoundContext_Private;
	struct ReverbVolume_Private;
	struct SoundBuffer;
	struct SoundSource;

	class SoundDevice
	{
	public:
		SoundDevice(subsystem::Subsystem* System);
		~SoundDevice();

		SoundDevice_Private* DeviceData = nullptr;
		std::map<string, SoundBuffer*> Buffers;
	private:
		void LoadExtensions();
	};

	class ReverbData
	{
	public:
		float Volume = 0;

		float Density = 1.0f;
		float Diffusion = 1.0f;
		float Gain = 0.315f;
		float HighFrequencyGain = 0.9f;
		float LowFrequencyGain = 1.0f;
		float DecayTime = 1.5f;
		float DecayHighFrequencyRatio = 0.85f;
		float DecayLowFrequencyRatio = 1.0f;
		float ReflectionsGain = 0.05f;
		float ReflectionsDelay = 0.007f;
		Vector3 ReflectionsPan = 0;
		float LateReverbGain = 1.0f;
		float LateReverbDelay = 0;
		Vector3 LateReverbPan = 0;
		float EchoTime = 0.2500f;
		float EchoDepth = 0;
		float ModulationTime = 0.2500f;
		float ModulationDepth = 0;
		float HighFrequencyAirAbsorptionGain = 1.0f;
		float HighFrequencyReference = 5000.0f;
		float LowFrequencyReference = 250.0f;
		float RoomRolloffFactor = 1.0f;
		int32 DecayHighFrequencyLimit = 1;

	};

	class SoundReverbVolume
	{
	public:
		Transform AtTransform;
		Transform AtInverse;

		ReverbData Data;
		float Falloff = 0.1f;

		void UpdateTransform()
		{
			AtInverse = AtTransform.Inverse();
		}

		debug::DebugBox* Box = nullptr;
		std::shared_ptr<ReverbVolume_Private> VolumeData;
	};

	class SoundContext
	{
	public:

		SoundContext(SoundDevice* Device);

		~SoundContext();

		SoundBuffer* LoadSoundEffect(string Path);
		SoundSource* CreateSoundSource(SoundBuffer* With);

		void SetSourcePosition(SoundSource* Source, Vector3 NewPosition);

		void SetSourceVelocity(SoundSource* Source, Vector3 NewVelocity);
		void PlaySource(SoundSource* Source, bool Loop, bool Is3D);
		void StopSource(SoundSource* Source);

		void Update(graphics::Camera* FromCamera, debug::DebugDraw* Debug);

		void FreeSoundSource(SoundSource* Source);
		void FreeSoundEffect(SoundBuffer* Buffer);

		void AddFileSource(SoundFileSource* Source);

		subsystem::Subsystem* System = nullptr;
		SoundContext_Private* SoundData = nullptr;

		SoundReverbVolume* AddReverbVolume(Transform At, ReverbData Data);
		void RemoveReverbVolume(SoundReverbVolume* Target);
		void UpdateReverbVolumeTransform(SoundReverbVolume* Target, Transform NewTransform);

		void Debug_ShowReverbAreas(debug::DebugDraw* DrawInstance);

		void SetShowDebugBounds(bool NewShowDebugBounds);
		const bool& GetShowDebugBounds() const
		{
			return ShowDebugBounds;
		}
		bool ShowDebugBoundsInEditor = true;
		void MarkReverbDirty();

	private:
		void MakeCurrent() const;

		std::list<SoundReverbVolume> ReverbVolumes;
		ReverbData CurrentReverb;
		std::shared_ptr<ReverbVolume_Private> CurrentReverbData;
		float CurrentReverbIntensity = 1.0f;

		struct MultiThreadData
		{
			bool Stop = false;
			std::mutex TreeBuildMutex;
			std::mutex SoundUpdateMutex;
			std::list<SoundReverbVolume> CurrentReverbVolumes;
		};

		std::shared_ptr<MultiThreadData> ThreadData = std::make_shared<MultiThreadData>();
		std::shared_ptr<graphics::BvhNode<SoundReverbVolume*>> ReverbTree;
		bool TreeModified = false;
		bool TreeCalculating = false;
		bool ShowDebugBounds = false;
		bool BoundsModified = false;

		void UpdateReverbVolumeTree();
		void UpdateReverb(Vector3 AtPosition);


		SoundDevice* Device = nullptr;
		std::vector<SoundFileSource*> Sources;
	};
}