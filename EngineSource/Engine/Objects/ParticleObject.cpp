#include "ParticleObject.h"

void engine::ParticleObject::Begin()
{
	Particles = new ParticleComponent();
	Attach(Particles);
	Particles->Load(AssetRef::Convert("Test.prt"));
}

void engine::ParticleObject::Update()
{
}
