#include "ParticleObject.h"

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
}

void engine::ParticleObject::Update()
{
}
