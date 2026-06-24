#include "CameraComponent.h"
#include <Engine/Objects/SceneObject.h>
#include <Engine/Scene.h>
#include <Engine/Engine.h>

using namespace engine;

engine::CameraComponent::CameraComponent()
{
	if (!Engine::IsPlaying)
	{
		return;
	}

	ComponentCamera = graphics::Camera(90);
	ComponentCamera.UseTransform = true;
}

engine::CameraComponent::~CameraComponent()
{
	Scene* ObjScene = GetRootObject()->GetScene();

	if (ObjScene->Graphics.UsedCamera == &ComponentCamera)
	{
		ObjScene->Graphics.UsedCamera = ObjScene->Graphics.SceneCamera;
	}
}

void engine::CameraComponent::Update()
{
	if (!Engine::IsPlaying)
	{
		return;
	}

	ComponentCamera.CameraTransform = WorldTransform;
	ComponentCamera.Aspect = GetRootObject()->GetScene()->Graphics.SceneCamera->Aspect;
	ComponentCamera.Update();
}

void engine::CameraComponent::Use()
{
	if (!Engine::IsPlaying  || !GetRootObject())
	{
		return;
	}

	GetRootObject()->GetScene()->Graphics.UsedCamera = &ComponentCamera;

	Update();
}

void engine::CameraComponent::SetFov(float NewFov)
{
	if (this->Fov != NewFov)
	{
		ComponentCamera.FOV = Rotation3::DegreeToRadian(NewFov);
		this->Fov = NewFov;
	}
}

float engine::CameraComponent::GetFov() const
{
	return Fov;
}

Vector3 engine::CameraComponent::ScreenToWorld(Vector2 Screen) const
{
	return ComponentCamera.ScreenToWorld(Screen);
}

Vector3 engine::CameraComponent::WorldToScreen(Vector3 World) const
{
	return ComponentCamera.WorldToScreen(World);
}
