#include "LightObject.h"
#include "Components/LightComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/PhysicsComponent.h"
#include <Engine/Engine.h>

#if EDITOR
#include <Editor/UI/EditorUI.h>

using namespace engine::editor;
#endif

void engine::LightObject::Begin()
{
#if EDITOR
	BillboardComponent* Billboard = nullptr;

	if (!Engine::IsPlaying)
	{
		Billboard = new BillboardComponent();
		Attach(Billboard);
		Billboard->LoadImage(AssetRef::FromPath(EditorUI::Asset("Light.png")));

		Billboard->SetColor(Color.Value);

		auto Collider = new PhysicsComponent();
		Attach(Collider);
		Collider->CreateSphere(physics::MotionType::Static, physics::Layer::Static, 0.25f);
	}

#define LIGHT_CAPTURE [this, Light, Billboard]
#else
#define LIGHT_CAPTURE [this, Light]
#endif

	auto Light = new LightComponent();
	Light->SetRange(Range.Value);
	Light->SetIntensity(Intensity.Value);
	Light->SetColor(Color.Value);
	Attach(Light);

	Range.OnChanged = LIGHT_CAPTURE{
		Light->SetRange(Range.Value);
	};
	Intensity.OnChanged = LIGHT_CAPTURE{
		Light->SetIntensity(Intensity.Value);
	};
	Color.OnChanged = LIGHT_CAPTURE{
		Light->SetColor(Color.Value);
#if EDITOR
		if (Billboard)
		{
			Billboard->SetColor(Color.Value);
		}
#endif
	};

	Color.AddHint(PropertyHint::Vec3Color);
}

void engine::LightObject::OnDestroyed()
{
}
