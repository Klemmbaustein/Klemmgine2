#include "CameraComponent.h"
#include <Engine/Objects/SceneObject.h>
#include <Engine/Scene.h>
#include <Engine/Engine.h>
#include <Engine/Input.h>
#include <Core/Log.h>

engine::CameraComponent::CameraComponent()
{
	if (!Engine::IsPlaying)
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
	if (!Engine::IsPlaying)
	{
		return;
	}

	ComponentCamera.CameraTransform = WorldTransform;
	ComponentCamera.Aspect = GetRootObject()->GetScene()->SceneCamera->Aspect;
	ComponentCamera.Update();
}

void engine::CameraComponent::Use()
{
	if (!Engine::IsPlaying)
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
