#pragma once
#include <Engine/Physics/Physics.h>
#include <unordered_map>
#include <utility>
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

class JoltJobSystemImpl;

namespace engine::internal
{
	class JoltInstance
	{
	public:
		static bool IsInitialized;
		static void InitJolt();

		JoltInstance();
		~JoltInstance();

		void Update();

		void AddBody(physics::PhysicsBody* Body, bool StartActive, bool StartCollisionEnabled);
		void RemoveBody(physics::PhysicsBody* Body);
		void CreateShape(physics::PhysicsBody* Body);
		void SetBodyPosition(physics::PhysicsBody* Body, Vector3 NewPosition);
		Vector3 GetBodyPosition(physics::PhysicsBody* Body);
		std::pair<Vector3, Rotation3> GetBodyPositionAndRotation(physics::PhysicsBody* Body);
		void SetBodyRotation(physics::PhysicsBody* Body, Vector3 NewRotation);
		void SetBodyPositionAndRotation(physics::PhysicsBody* Body, Vector3 NewPosition, Rotation3 NewRotation);
		void ScaleBody(physics::PhysicsBody* Body, Vector3 ScaleFactor);
		void SetBodyActive(physics::PhysicsBody* Body, bool IsActive);
		void SetBodyCollisionEnabled(physics::PhysicsBody* Body, bool IsCollisionEnabled);

		void PreLoadMesh(GraphicsModel* Mesh);

		std::vector<physics::HitResult> CollisionTest(Transform At, physics::PhysicsBody* Body, physics::Layer Layers, std::set<SceneObject*> ObjectsToIgnore);
		std::vector<physics::HitResult> ShapeCastBody(physics::PhysicsBody* Body, Transform StartPos, Vector3 EndPos, physics::Layer Layers, std::set<SceneObject*> ObjectsToIgnore);
		physics::HitResult LineCast(Vector3 Start, Vector3 End, physics::Layer Layers, std::set<SceneObject*> ObjectsToIgnore);

		struct PhysicsBodyInfo
		{
			JPH::BodyID ID;
			physics::PhysicsBody* Body = nullptr;
		};

		std::unordered_map<JPH::BodyID, PhysicsBodyInfo> Bodies;
		JPH::BodyInterface* JoltBodyInterface = nullptr;

	private:

		JPH::MeshShape* CreateNewMeshShape(GraphicsModel* From, bool ExtraReference = true);

		void UnloadMesh(GraphicsModel* Mesh);
		JPH::BodyCreationSettings CreateJoltShapeFromBody(physics::PhysicsBody* Body);
		struct PhysicsMesh
		{
			JPH::MeshShape* Shape = nullptr;
			uint64 References = 0;
		};

		std::unordered_map<GraphicsModel*, PhysicsMesh> LoadedMeshes;

		JPH::PhysicsSystem* System = nullptr;
		static JPH::TempAllocatorImpl* TempAllocator;
		static JoltJobSystemImpl* JobSystem;
	};
}