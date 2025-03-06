#include "PlayerObject.h"
#include <Core/Log.h>
#include <Engine/Input.h>
#include <Engine/Stats.h>
#include <Editor/Editor.h>
#include "Components/MeshComponent.h"

void engine::PlayerObject::Begin()
{
	Movement = new MoveComponent();
	Attach(Movement);

	Cam = new CameraComponent();
	Attach(Cam);
	Cam->Use();
	Cam->Position.Y = 0.4f;

	auto Mesh = new MeshComponent();
	Attach(Mesh);
	Mesh->Scale = Vector3(0.4f, 1.0f, 0.4f);
	Mesh->Load(PlayerModel.Value);
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
		Move(CameraTransform.Forward() * Speed);
	}
	if (input::IsKeyDown(input::Key::s))
	{
		Move(-CameraTransform.Forward() * Speed);
	}
	if (input::IsKeyDown(input::Key::d))
	{
		Move(CameraTransform.Right() * Speed);
	}
	if (input::IsKeyDown(input::Key::a))
	{
		Move(-CameraTransform.Right() * Speed);
	}

	if (input::IsKeyDown(input::Key::SPACE))
	{
		Movement->Jump();
	}
}

void engine::PlayerObject::Move(Vector3 Direction)
{
	Movement->AddMovementInput((Direction * Vector3(1, 0, 1)).Normalize());
}