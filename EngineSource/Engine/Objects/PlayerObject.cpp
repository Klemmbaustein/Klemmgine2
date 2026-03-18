#include "PlayerObject.h"
#include <Engine/Input.h>
#include <Engine/Stats.h>
#include <Engine/Engine.h>
#include "Components/MeshComponent.h"

void engine::PlayerObject::Begin()
{
	Movement = new MoveComponent();
	Attach(Movement);

	Cam = new CameraComponent();
	Attach(Cam);
	Cam->Use();
	Cam->SetFov(Fov.Value);

	this->Fov.OnChanged = [this] {
		Cam->SetFov(Fov.Value);
	};

	Cam->SetPosition(Vector3(0, 0.75f, 0));

	auto Mesh = new MeshComponent();
	Attach(Mesh);
	Mesh->SetScale(Vector3(0.5f, 1.0f, 0.5f));
	Mesh->Load(PlayerModel.Value);
}

void engine::PlayerObject::Update()
{
	if (!Engine::IsPlaying)
		return;

	Cam->SetRotation(Cam->GetRotation().EulerVector() - Vector3(input::MouseMovement.Y, 0, 0));

	this->Rotation.Y -= input::MouseMovement.X;

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