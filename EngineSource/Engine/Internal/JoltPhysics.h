#pragma once
#include <Engine/Physics/Physics.h>
#include <unordered_map>
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/ShapeCast.h>

namespace engine::internal
{
	class JoltInstance
	{
	public:
		static void InitJolt();

		JoltInstance();

		void Update();

		void AddBody(engine::physics::PhysicsBody* Body, bool StartActive, bool StartCollisionEnabled);
		void RemoveBody(engine::physics::PhysicsBody* Body);
		void CreateShape(engine::physics::PhysicsBody* Body);
		void SetBodyPosition(engine::physics::PhysicsBody* Body, Vector3 NewPosition);
		void SetBodyRotation(engine::physics::PhysicsBody* Body, Vector3 NewRotation);
		void SetBodyPositionAndRotation(engine::physics::PhysicsBody* Body, Vector3 NewPosition, Rotation3 NewRotation);
		void ScaleBody(engine::physics::PhysicsBody* Body, Vector3 ScaleFactor);
		void SetBodyActive(engine::physics::PhysicsBody* Body, bool IsActive);
		void SetBodyCollisionEnabled(engine::physics::PhysicsBody* Body, bool IsCollisionEnabled);

		engine::physics::HitResult LineCast(Vector3 Start, Vector3 End, engine::physics::Layer Layers, std::set<SceneObject*> ObjectsToIgnore);

		struct PhysicsBodyInfo
		{
			JPH::BodyID ID;
			JPH::Shape* BodyShape = nullptr;
			engine::physics::PhysicsBody* Body = nullptr;
			GraphicsModel* ReferencedModel = nullptr;
		};
		
		std::unordered_map<JPH::BodyID, PhysicsBodyInfo> Bodies;

	private:
		JPH::BodyCreationSettings CreateJoltShapeFromBody(engine::physics::PhysicsBody* Body);
		struct PhysicsMesh
		{
			JPH::MeshShape* Shape = nullptr;
			uint64 References = 0;
		};

		std::unordered_map<GraphicsModel*, PhysicsMesh> LoadedMeshes;

		JPH::PhysicsSystem* System = nullptr;
		JPH::BodyInterface* JoltBodyInterface = nullptr;
		JPH::TempAllocatorImpl* TempAllocator = nullptr;
		JPH::JobSystemThreadPool* JobSystem = nullptr;
	};
}