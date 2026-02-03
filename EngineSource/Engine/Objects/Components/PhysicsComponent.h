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

		void CreateSphere(physics::MotionType Movability, physics::Layer Layers = physics::Layer::Dynamic, bool StartEnabled = true);
		void CreateBox(physics::MotionType Movability, physics::Layer Layers = physics::Layer::Dynamic, bool StartEnabled = true);
		void CreateCapsule(physics::MotionType Movability, physics::Layer Layers = physics::Layer::Dynamic, bool StartEnabled = true);

		std::vector<physics::HitResult> ShapeCast(Transform Start, Vector3 End, physics::Layer Layers, std::set<SceneObject*> ObjectsToIgnore = {});

		std::vector<physics::HitResult> CollisionTest(physics::Layer Layers, std::set<SceneObject*> ObjectsToIgnore = {});

		void SetCollisionEnabled(bool NewEnabled);
		bool GetCollisionEnabled() const;

		void SetActive(bool NewActive);
		bool GetActive() const;
		void Update() override;

	private:
		Transform LastTransform;
		bool Added = false;
		void Clear();
		physics::PhysicsBody* Body = nullptr;
	};
}