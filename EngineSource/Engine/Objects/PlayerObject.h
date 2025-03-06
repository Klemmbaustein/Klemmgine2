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

		ENGINE_OBJECT(PlayerObject, "Game");

		ObjProperty<AssetRef> PlayerModel = ObjProperty<AssetRef>("Model", "cube.kmdl"_asset, this);

		void Begin() override;
		void Update() override;

		void Move(Vector3 Direction);
	};
}
