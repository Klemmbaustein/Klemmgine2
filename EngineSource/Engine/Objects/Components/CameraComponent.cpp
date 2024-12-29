#include "CameraComponent.h"
#include <Engine/Objects/SceneObject.h>
#include <Engine/Scene.h>
#include <Engine/Editor/Editor.h>
#include <Engine/Input.h>
#include <Engine/Log.h>

engine::CameraComponent::CameraComponent()
{
	if (editor::IsActive())
	{
		return;
	}

	ComponentCamera = graphics::Camera(Rotation3::PI / 2);
	ComponentCamera.UseTransform = true;
}

engine::CameraComponent::~CameraComponent()
{
	Scene* ObjScene = GetRootObject()->GetScene();

	if (ObjScene->UsedCamera == &ComponentCamera)
	{
		ObjScene->UsedCamera = ObjScene->SceneCamera;
	}
}

void engine::CameraComponent::Update()
{
	if (editor::IsActive())
	{
		return;
	}

	ComponentCamera.CameraTransform = WorldTransform;
	ComponentCamera.Update();
}

void engine::CameraComponent::Use()
{
	if (editor::IsActive())
	{
		return;
	}

	GetRootObject()->GetScene()->UsedCamera = &ComponentCamera;
}

void engine::CameraComponent::SetFov(float NewFov)
{
	NewFov = Rotation3::DegreeToRadian(NewFov);
	if (this->Fov != NewFov)
	{
		ComponentCamera.FOV = NewFov;
		this->Fov = NewFov;
	}
}

float engine::CameraComponent::GetFov() const
{
	return Rotation3::RadianToDegree(Fov);
}
