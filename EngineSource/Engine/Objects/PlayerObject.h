#pragma once
#include "SceneObject.h"
#include "Components/CameraComponent.h"
#include "Components/MoveComponent.h"

namespace engine
{
	class PlayerObject : public SceneObject
	{
	public:

		CameraComponent* Cam = nullptr;
		MoveComponent* Movement = nullptr;

		ENGINE_OBJECT(PlayerObject, "Engine");

		ObjProperty<AssetRef> PlayerModel = ObjProperty<AssetRef>("Model", "Cube.kmdl"_asset, this);
		ObjProperty<float> Fov = ObjProperty<float>("FOV", 70.0f, this);

		void Begin() override;
		void Update() override;

		void Move(Vector3 Direction);
	};
}
