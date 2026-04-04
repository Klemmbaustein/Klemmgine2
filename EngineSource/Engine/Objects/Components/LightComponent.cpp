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

void engine::LightComponent::SetRange(float NewRange)
{
	this->Range = NewRange;
	UpdateLight(WorldTransform.ApplyTo(0));
}

void engine::LightComponent::SetIntensity(float NewIntensity)
{
	this->Intensity = NewIntensity;
	UpdateLight(WorldTransform.ApplyTo(0));
}

void engine::LightComponent::SetColor(Vector3 NewColor)
{
	this->Color = NewColor;
	UpdateLight(WorldTransform.ApplyTo(0));
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
		.Intensity = Intensity,
		.Range = Range,
		.Color = Color,
		.Position = At,
		});
}
