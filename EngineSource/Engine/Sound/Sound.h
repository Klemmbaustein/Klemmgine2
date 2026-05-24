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

		SoundReverbVolume* AddReverbVolume(Transform At);
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