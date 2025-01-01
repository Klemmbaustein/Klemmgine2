#include "PlayerObject.h"
#include <Engine/Log.h>
#include <Engine/Input.h>
#include <Engine/Stats.h>
#include <Engine/Editor/Editor.h>

void engine::PlayerObject::Begin()
{

	Collider = new PhysicsComponent();
	Attach(Collider);
	Collider->Scale = 0.5f;
	Collider->CreateSphere(physics::MotionType::Static, physics::Layer::Static, false);

	Cam = new CameraComponent();
	Collider->Attach(Cam);
	Cam->Use();
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
}

void engine::PlayerObject::Move(Vector3 Direction)
{
	auto hit = Collider->ShapeCast(ObjectTransform, Position + Direction, physics::Layer::Static);

	if (hit.Hit)
	{
		Position += hit.Normal * Vector3::Dot(hit.Normal, -Direction.Normalize()) * Direction.Length();
	}
	Position += Direction;
}
