#pragma once
#include "ObjectComponent.h"
#include <Engine/Physics/Physics.h>

namespace engine
{
	class PhysicsComponent : public ObjectComponent
	{
	public:
		PhysicsComponent();
		virtual ~PhysicsComponent() override;

		void CreateSphere(physics::MotionType Movability, physics::Layer Layers,
			float Scale = 1.0f, bool StartEnabled = true);
		void CreateBox(physics::MotionType Movability, physics::Layer Layers,
			Vector3 Scale = Vector3(1), bool StartEnabled = true);
		void CreateCapsule(physics::MotionType Movability, physics::Layer Layers,
			Vector2 Scale, bool StartEnabled = true);

		std::vector<physics::HitResult> ShapeCast(Transform Start, Vector3 End, physics::Layer Layers,
			std::set<SceneObject*> ObjectsToIgnore = {});

		std::vector<physics::HitResult> CollisionTest(physics::Layer Layers, std::set<SceneObject*> ObjectsToIgnore = {});

		void SetCollisionEnabled(bool NewEnabled);
		bool GetCollisionEnabled() const;

		void SetActive(bool NewActive);
		bool GetActive() const;
		void Update() override;

		void SetVelocity(Vector3 NewVelocity);

		bool UpdateTransform(bool IsDirty) override;

		std::function<void()> OnBeginOverlap;

	private:
		Vector3 Offset;
		Transform LastTransform;
		bool Added = false;
		bool IsPhysicsSimulated = false;
		void Clear();
		physics::PhysicsBody* Body = nullptr;
	};
}