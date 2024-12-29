#pragma once
#include "SceneObject.h"
#include "Components/CameraComponent.h"

namespace engine
{
	class PlayerObject : public SceneObject
	{
	public:

		CameraComponent* Cam = nullptr;

		ENGINE_OBJECT(PlayerObject, "Game");

		void Begin() override;
		void Update() override;
	};
}