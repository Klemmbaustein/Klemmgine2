#pragma once
#include "SceneObject.h"
#include "Components/MeshComponent.h"
#include "Components/CollisionComponent.h"

namespace engine
{
	class MeshObject : public SceneObject
	{
	public:

		ENGINE_OBJECT(MeshObject, "Engine");

		ObjProperty<AssetRef> ModelName = ObjProperty<AssetRef>("Model", ".kmdl"_asset, this);

		MeshComponent* Mesh = nullptr;
		CollisionComponent* Collider = nullptr;

		void LoadMesh(AssetRef File);

		void Begin() override;
		void OnDestroyed() override;
	};
}