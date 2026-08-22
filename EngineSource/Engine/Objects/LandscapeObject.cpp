#include "LandscapeObject.h"

void engine::LandscapeObject::Begin()
{
	Component = new LandscapeComponent();

	auto Asset = "Grass.kmt"_asset;

	if (Asset.Exists())
	{
		Component->LandscapeMaterial = new graphics::Material(Asset);
	}
	else
	{
		Component->LandscapeMaterial = new graphics::Material("Grass.kbm"_asset);
	}

	this->Attach(Component);
}

void engine::LandscapeObject::OnDestroyed()
{
}
