#include "LightObject.h"
#include "Components/LightComponent.h"

void engine::LightObject::Begin()
{
	Attach(new LightComponent());
}

void engine::LightObject::OnDestroyed()
{
}
