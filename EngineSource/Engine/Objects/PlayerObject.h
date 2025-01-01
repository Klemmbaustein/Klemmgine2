#pragma once
#include "SceneObject.h"
#include "Components/CameraComponent.h"
#include "Components/PhysicsComponent.h"

namespace engine
{
	class PlayerObject : public SceneObject
	{
	public:

		CameraComponent* Cam = nullptr;
		PhysicsComponent* Collider = nullptr;

		ENGINE_OBJECT(PlayerObject, "Game");

		void Begin() override;
		void Update() override;

		void Move(Vector3 Direction);
	};
}