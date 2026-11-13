#pragma once
#include <Core/Vector.h>
#include <Core/File/SerializedData.h>
#include <Engine/File/AssetRef.h>
#include <Engine/Graphics/Material.h>
#include <Engine/Graphics/Model.h>
#include <mutex>
#include <list>

namespace engine::graphics
{
	class ParticleEmitterData : Serializable
	{
	public:
		ParticleEmitterData();

		Vector3 StartPosition;
		Vector3 StartRandom;

		Vector3 Velocity = Vector3(0, 1, 0);
		Vector3 VelocityRandom;

		Vector3 Acceleration;

		float Rotation = 0.0f;
		float RotationVelocity = 0.0f;
		float RotationVelocityRandom = 0.0f;

		float StartScale = 0.25f;
		float EndScale = 0.0f;
		float Lifetime = 1.0f;
		float SpawnDelay = 0.1f;
		int32 SpawnCount = 0;

		graphics::Material* Material = nullptr;
		AssetRef MaterialFile = AssetRef::EmptyAsset("kmt");

		void ReloadMaterial();

		// Inherited via Serializable
		SerializedValue Serialize() override;
		void DeSerialize(SerializedValue* From) override;
	};

	class ParticleInstance
	{
	public:
		Vector3 Position;
		Vector3 Velocity;
		float Rotation = 0.0f;
		float RotationVelocity = 0.0f;
		float Scale = 0;
		float Lifetime = 1.0f;
	};

	class ParticleEmitter
	{
	public:
		ParticleEmitterData Data;

		graphics::Model* InstancesModel = nullptr;
		graphics::VertexArrayData* InstancesArray = nullptr;

		std::vector<Transform> Transforms;
		std::mutex EmitterMutex;

		bool IsProcessingEffect = false;

		size_t MainThreadInstanceCount = 0;
		std::atomic<int32> SpawnCount = 0;
		std::atomic<int32> CurrentInstanceCount;

		bool IsPlaying() const;

		BoundingBox Bounds;
		BoundingBox MainThreadBounds;

		void RunStep(float Delta, Transform ViewTransform, bool AutomaticBoundingVolume);

		void Reset();

		std::vector<ParticleInstance>& GetInstances()
		{
			return CurrentActiveInstances ? Instances1 : Instances2;
		}

	private:
		float NewParticleTimer = 0;
		bool CurrentActiveInstances = false;
		std::vector<ParticleInstance>& GetInactiveInstances()
		{
			return CurrentActiveInstances ? Instances2 : Instances1;
		}

		std::vector<ParticleInstance> Instances1;
		std::vector<ParticleInstance> Instances2;
	};

	class ParticleData : Serializable
	{
	public:
		ParticleData(AssetRef FromFile);
		ParticleData();

		std::list<ParticleEmitter> Emitters;
		std::mutex EmittersMutex;

		bool AutomaticBoundingVolume = true;

		// Inherited via Serializable
		SerializedValue Serialize() override;
		void DeSerialize(SerializedValue* From) override;
	};
}