#pragma once
#include <Engine/Objects/SceneObject.h>
#include <Engine/Objects/Components/ParticleComponent.h>

namespace engine
{
	class ParticleObject : public SceneObject
	{
	public:
		ENGINE_OBJECT(ParticleObject, "Engine/SceneObject", "Engine");

		void Begin() override;
		void Update() override;

		PROPERTY(AssetRef, ParticleFile, =, AssetRef::EmptyAsset("prt"));

		ParticleComponent* Particles = nullptr;
	};
}