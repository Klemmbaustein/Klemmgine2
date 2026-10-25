#pragma once
#include <Core/Vector.h>
#include <Core/File/SerializedData.h>
#include <Engine/File/AssetRef.h>
#include <Engine/Graphics/Material.h>
#include <Engine/Graphics/Model.h>

namespace engine::graphics
{
	class ParticleEmitterData : Serializable
	{
	public:
		Vector3 StartPosition;
		Vector3 StartRandom;

		Vector3 Velocity = Vector3(0, 1, 0);
		Vector3 VelocityRandom;

		Vector3 Acceleration;

		float Rotation = 45.0f;
		float RotationVelocity = 360.0f;

		float StartScale = 0.25f;
		float EndScale = 1.0f;
		float Lifetime = 1.0f;

		graphics::Material* Material = nullptr;

		// Inherited via Serializable
		SerializedValue Serialize() override;
		void DeSerialize(SerializedValue* From) override;
	};

	class ParticleInstance
	{
	public:
		Vector3 Position;
		Vector3 Velocity;
		float Rotation = 0;
		float RotationVelocity;
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

		bool IsRunning = false;

		size_t MainThreadInstanceCount = 0;

		BoundingBox Bounds;

		void RunStep(float Delta, Transform ViewTransform);

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

		std::vector<ParticleEmitter> Emitters;

		// Inherited via Serializable
		SerializedValue Serialize() override;
		void DeSerialize(SerializedValue* From) override;
	};
}