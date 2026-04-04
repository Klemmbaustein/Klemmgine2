#include "LightObject.h"
#include "Components/LightComponent.h"

void engine::LightObject::Begin()
{
	auto Light = new LightComponent();
	Light->SetRange(Range.Value);
	Light->SetIntensity(Intensity.Value);
	Light->SetColor(Color.Value);
	Attach(Light);

	Range.OnChanged = [this, Light] {
		Light->SetRange(Range.Value);
	};
	Intensity.OnChanged = [this, Light] {
		Light->SetIntensity(Intensity.Value);
	};
	Color.OnChanged = [this, Light] {
		Light->SetColor(Color.Value);
	};

	Color.AddHint(PropertyHint::Vec3Color);
}

void engine::LightObject::OnDestroyed()
{
}
