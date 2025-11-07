#include "LandscapeObject.h"

void engine::LandscapeObject::Begin()
{
	auto c = new LandscapeComponent();

	auto Asset = "Grass.kmt"_asset;

	if (Asset.Exists())
	{
		c->LandscapeMaterial = new graphics::Material(Asset);
	}
	else
	{
		c->LandscapeMaterial = new graphics::Material("Grass.kbm"_asset);
	}

	this->Attach(c);
}

void engine::LandscapeObject::OnDestroyed()
{
}
