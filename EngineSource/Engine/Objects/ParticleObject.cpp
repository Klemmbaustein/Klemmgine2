#include "ParticleObject.h"
#include <Engine/Objects/Components/PhysicsComponent.h>
#include <Engine/Engine.h>
#include <Engine/Scene.h>

void engine::ParticleObject::Begin()
{
	Particles = new ParticleComponent();
	Attach(Particles);

	if (ParticleFile.Value.Exists())
	{
		Particles->Load(ParticleFile.Value);
	}

	ParticleFile.OnChanged = [this] {
		Particles->Load(ParticleFile.Value);
	};
#if EDITOR
	if (!Engine::IsPlaying && GetScene()->Physics.Active)
	{
		auto Collider = new PhysicsComponent();
		Attach(Collider);
		Collider->CreateSphere(physics::MotionType::Static, physics::Layer::Static, 0.33f);
	}

#endif
}

void engine::ParticleObject::Update()
{
}
