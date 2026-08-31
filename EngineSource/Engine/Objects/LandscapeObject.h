#pragma once
#include "SceneObject.h"
#include "Components/LandscapeComponent.h"

namespace engine
{
	class LandscapeObject : public SceneObject
	{
	public:

		void Begin() override;
		void OnDestroyed() override;

		ENGINE_OBJECT(LandscapeObject, "Engine/SceneObject", "Engine");

		ObjProperty<AssetRef> Material = ObjProperty<AssetRef>("Material", AssetRef::EmptyAsset("kmt"), this);
		PROPERTY(float, LodFalloff, =, 1.5f);

		PROPERTY(AssetRef, HeightMap, =, AssetRef::EmptyAsset("hmp"));

		LandscapeComponent* Component = nullptr;

		void OnSaved() override;

	private:
	};
}