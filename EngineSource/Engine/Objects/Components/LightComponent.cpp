#include "LightComponent.h"
#include <Engine/Objects/SceneObject.h>
#include <Engine/Scene.h>

engine::LightComponent::LightComponent()
{
}

engine::LightComponent::~LightComponent()
{
	auto Root = RootObject ? RootObject->GetScene() : nullptr;

	if (!Root)
	{
		return;
	}

	if (LightData)
	{
		Root->Graphics.Lights.RemoveLight(LightData);
	}
}

void engine::LightComponent::Update()
{
	Vector3 NewPosition = WorldTransform.ApplyTo(0);

	if (NewPosition != this->LastPosition)
	{
		UpdateLight(NewPosition);
	}
}

void engine::LightComponent::OnAttached()
{
	Vector3 NewPosition = WorldTransform.ApplyTo(0);

	UpdateLight(NewPosition);
}

void engine::LightComponent::UpdateLight(Vector3 At)
{
	auto Root = RootObject ? RootObject->GetScene() : nullptr;

	if (!Root)
	{
		return;
	}

	if (LightData)
	{
		Root->Graphics.Lights.RemoveLight(LightData);
	}

	this->LastPosition = At;

	LightData = Root->Graphics.Lights.AddLight(graphics::Light{
		.Intensity = 1,
		.Range = 2.0f,
		.Color = 1,
		.Position = At,
		});
}
