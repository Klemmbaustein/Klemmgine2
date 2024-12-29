#include "PlayerObject.h"
#include <Engine/Log.h>
#include <Engine/Input.h>
#include <Engine/Stats.h>
#include <Engine/Editor/Editor.h>

void engine::PlayerObject::Begin()
{
	Cam = new CameraComponent();
	Attach(Cam);
	Cam->Use();
	Cam->Position = Vector3(0, 0, 0);
}

void engine::PlayerObject::Update()
{
	if (editor::IsActive())
		return;

	Cam->Rotation = Cam->Rotation.EulerVector() - Vector3(input::MouseMovement.Y, input::MouseMovement.X, 0) * 70;

	float Speed = 5 * stats::DeltaTime;

	Transform CameraTransform = Cam->GetWorldTransform();

	if (input::IsKeyDown(input::Key::w))
	{
		Position += CameraTransform.Forward() * Speed;
	}
	if (input::IsKeyDown(input::Key::s))
	{
		Position -= CameraTransform.Forward() * Speed;
	}
	if (input::IsKeyDown(input::Key::d))
	{
		Position += CameraTransform.Right() * Speed;
	}
	if (input::IsKeyDown(input::Key::a))
	{
		Position -= CameraTransform.Right() * Speed;
	}
}
