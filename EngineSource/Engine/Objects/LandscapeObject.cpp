#include "LandscapeObject.h"

void engine::LandscapeObject::Begin()
{
	auto c = new LandscapeComponent();
	c->LandscapeMaterial = new graphics::Material("Grass.kmt"_asset);
	this->Attach(c);
}

void engine::LandscapeObject::OnDestroyed()
{
}
