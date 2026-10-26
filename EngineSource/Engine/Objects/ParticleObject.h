#pragma once
#include <Engine/Objects/SceneObject.h>
#include <Engine/Objects/Components/ParticleComponent.h>

namespace engine
{
	class ParticleObject : SceneObject
	{
	public:
		ENGINE_OBJECT(ParticleObject, "Engine/SceneObject", "Engine");

		void Begin() override;
		void Update() override;

		ParticleComponent* Particles = nullptr;
	};
}