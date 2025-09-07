#include "MoveComponent.h"
#include <Engine/Objects/SceneObject.h>
#include <Engine/Engine.h>
#include <Engine/Stats.h>

using namespace engine;

engine::MoveComponent::MoveComponent()
{
}

void engine::MoveComponent::OnAttached()
{
	Collider = new PhysicsComponent();
	Attach(Collider);
	Collider->Scale = Vector3(this->ColliderSize.X, this->ColliderSize.Y, 1);
	Collider->CreateCapsule(physics::MotionType::Static, physics::Layer::Static, false);
}

void engine::MoveComponent::Update()
{
	if (!Engine::IsPlaying || !this->Active)
	{
		return;
	}

	Transform WorldTransform = GetWorldTransform();

	InputDirection.Y = 0;
	if (InputDirection.Length() > 1)
	{
		InputDirection = InputDirection.Normalize();
	}

	float AccelModifier = GroundedTimer ? 1 : AirAccelMultiplier;

	MovementVelocity += Vector2(InputDirection.X, InputDirection.Z) * stats::DeltaTime * Acceleration * AccelModifier;

	float InputLength = InputDirection.Length();
	float MovementLength = MovementVelocity.Length();

	if (MovementLength > MaxSpeed * InputLength)
	{
		if (MovementLength > MaxSpeed && InputLength >= 0.95f)
		{
			MovementVelocity = MovementVelocity.Normalize() * MaxSpeed;
		}
		else
		{
			MovementLength = std::max(MovementLength - Deceleration * stats::DeltaTime * AccelModifier, 0.0f);
			MovementVelocity = MovementVelocity.Normalize() * MovementLength;
		}
	}

	Vector3 Position, Scale;
	Rotation3 Rotation;
	WorldTransform.Decompose(Position, Rotation, Scale);

	Vector3 MoveDir = Vector3(MovementVelocity.X, 0, MovementVelocity.Y).ProjectToPlane(0, GroundNormal) * stats::DeltaTime;
	LastMoveSuccessful = true;
	Vector3 Moved = TryMove(MoveDir, MoveDir, Position, false);
	RootObject->Position += Moved;


	MoveDir = Vector3(0, (VerticalVelocity - 1.0f + (Jumping ? JumpHeight : 0)) * stats::DeltaTime, 0);
	Vector3 GravityMovement = TryMove(MoveDir, MoveDir, Position, true);

	RootObject->Position += GravityMovement;

	if (GroundedTimer == 0)
	{
		VerticalVelocity -= stats::DeltaTime * Gravity;
	}
	else
	{
		VerticalVelocity = 0;
		Jumping = false;
	}

	InputDirection = 0;
}

Vector3 engine::MoveComponent::TryMove(Vector3 Direction, Vector3 InitialDirection, Vector3 Pos, bool GravityPass, uint32 Depth)
{
	float Distance = Direction.Length() + 0.001f;

	auto Hits = Collider->ShapeCast(
		Transform(Pos, 0, 1),
		Pos + Direction.Normalize() * Distance,
		physics::Layer::Static,
		{ RootObject }
	);

	if (Depth >= MoveMaxDepth)
	{
		if (!GravityPass)
		{
			LastMoveSuccessful = false;
			LastHitNormal = Direction.Normalize();
		}
		return 0;
	}

	if (!Hits.size())
	{
		if (GravityPass && GroundedTimer > 0)
		{
			GroundedTimer--;
		}
		else if (GravityPass)
		{
			GroundNormal = Vector3(0, 1, 0);
		}
		return Direction;
	}

	Vector3 HitNormal = Vector3(0, 0, 0);
	float MinDistance = INFINITY;

	bool HitStep = false;

	Vector3 AvgPos = 0;

	for (auto& i : Hits)
	{
		HitNormal += i.Normal * i.Depth;
		MinDistance = std::min(i.Distance, MinDistance);
		AvgPos += i.ImpactPoint;
	}
	HitNormal = HitNormal.Normalize();
	AvgPos = AvgPos / Vector3((float)Hits.size());

	if (HitNormal.Length() == 0)
	{
		HitNormal = 0;
		for (auto& i : Hits)
		{
			HitNormal += i.Normal;
		}
		HitNormal = HitNormal.Normalize();
	}

	float Angle = Vector3::Dot(HitNormal, Vector3(0, 1, 0));

	float StepSize = ColliderSize.Y * 0.85f;
	if (!GravityPass && GetVelocity().Y > -5.0f)
	{
		Vector3 NewDir = (-HitNormal * Vector3(1, 0, 1)).Normalize() * 1;
		Vector3 TestPos = Pos + Vector3(0, StepSize, 0);

		auto Hits = Collider->ShapeCast(
			Transform(TestPos, 0, Vector3(ColliderSize.X, ColliderSize.Y, ColliderSize.X)),
			TestPos + NewDir,
			physics::Layer::Static,
			{ RootObject }
		);

		HitStep = AvgPos.Y - Pos.Y < -StepSize && !Hits.size();
		if (HitStep)
		{
			HitNormal = Vector3(0, 1, 0);
		}
	}

	float AbsoluteDistance = MinDistance * Direction.Length();
	Vector3 SnapToSurface = Direction.Normalize() * (AbsoluteDistance - 0.001f);
	Vector3 LeftOver = Direction - SnapToSurface;

	float Length = LeftOver.Length();
	LeftOver = LeftOver.ProjectToPlane(0, HitNormal);
	LeftOver = LeftOver.Normalize() * Length;
	if (Angle > 0.75f)
	{
		if (GravityPass)
		{
			GroundedTimer = 5;
			GroundNormal = HitNormal;
			StoodOn = Hits[0].HitComponent;
			return SnapToSurface;
		}
	}
	else
	{
		if (Angle < -0.5)
		{
			Jumping = false;
			VerticalVelocity = 0;
		}

		float Scale = 1 - Vector3::Dot(
			Vector3(HitNormal.X, 0, HitNormal.Z).Normalize(),
			Vector3(-InitialDirection.X, 0, -InitialDirection.Z).Normalize()
		);
		LeftOver = LeftOver * Scale;

		LastMoveSuccessful = false;
		LastHitNormal = HitNormal;
	}

	if (!GravityPass)
	{
		SnapToSurface += HitNormal * stats::DeltaTime * (HitStep ? 5.0f : 1.0f) * StepSize;
	}

	return SnapToSurface + TryMove(LeftOver, InitialDirection, Pos + SnapToSurface, GravityPass, Depth + 1);
}

bool engine::MoveComponent::GetIsOnGround() const
{
	return GroundedTimer > 0;
}

Vector3 engine::MoveComponent::GetVelocity() const
{
	return Vector3(MovementVelocity.X, VerticalVelocity + (Jumping ? JumpHeight : 0), MovementVelocity.Y);
}

void engine::MoveComponent::SetVelocity(Vector3 NewVelocity)
{
	MovementVelocity.X = NewVelocity.X;
	MovementVelocity.Y = NewVelocity.Z;
	VerticalVelocity = NewVelocity.Y - (Jumping ? JumpHeight : 0);
}

void engine::MoveComponent::AddMovementInput(Vector3 Direction)
{
	InputDirection += Direction;
}

void engine::MoveComponent::Jump()
{
	if (GetIsOnGround())
	{
		Jumping = true;
		GroundedTimer = 0;
	}
}
