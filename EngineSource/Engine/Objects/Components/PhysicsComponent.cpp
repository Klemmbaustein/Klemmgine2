#include "PhysicsComponent.h"
#include <Engine/Scene.h>
#include <Core/Log.h>
using namespace engine;

engine::PhysicsComponent::PhysicsComponent()
{
}

engine::PhysicsComponent::~PhysicsComponent()
{
	Clear();
}

void engine::PhysicsComponent::CreateSphere(physics::MotionType Movability, physics::Layer Layers, bool StartEnabled)
{
	if (Body)
	{
		Clear();
	}

	Body = new physics::SphereBody(Position,
		Rotation,
		(Scale.X + Scale.Y + Scale.Z) / 3,
		Movability,
		Layers,
		this);

	if (StartEnabled)
	{
		GetRootObject()->GetScene()->Physics.AddBody(Body, true, true);
		Added = true;
	}
	else
	{
		Body->IsCollisionEnabled = false;
	}
}

void engine::PhysicsComponent::CreateBox(physics::MotionType Movability, physics::Layer Layers, bool StartEnabled)
{
	if (Body)
	{
		Clear();
	}

	Body = new physics::BoxBody(Position,
		Rotation,
		Scale,
		Movability,
		Layers,
		this);

	if (StartEnabled)
	{
		GetRootObject()->GetScene()->Physics.AddBody(Body, true, true);
		Added = true;
	}
	else
	{
		Body->IsCollisionEnabled = false;
	}
}

void engine::PhysicsComponent::CreateCapsule(physics::MotionType Movability, physics::Layer Layers, bool StartEnabled)
{
	if (Body)
	{
		Clear();
	}

	Body = new physics::CapsuleBody(Position,
		Rotation,
		Vector2(Scale.X, Scale.Y),
		Movability,
		Layers,
		this);

	GetRootObject()->GetScene()->Physics.AddBody(Body, true, StartEnabled);
	Added = true;
}

std::vector<physics::HitResult> engine::PhysicsComponent::ShapeCast(Transform Start, Vector3 End, physics::Layer Layers, std::set<SceneObject*> ObjectsToIgnore)
{
	if (!Body)
	{
		return {};
	}
	return Body->ShapeCast(Start, End, Layers, ObjectsToIgnore);
}

std::vector<physics::HitResult> engine::PhysicsComponent::CollisionTest(physics::Layer Layers, std::set<SceneObject*> ObjectsToIgnore)
{
	if (!Body)
	{
		return {};
	}
	return Body->CollisionTest(GetWorldTransform(), Layers, ObjectsToIgnore);
}

void engine::PhysicsComponent::SetCollisionEnabled(bool NewEnabled)
{
	if (!Body)
		return;

	if (NewEnabled && !Added)
	{
		GetRootObject()->GetScene()->Physics.AddBody(Body, Body->IsActive, NewEnabled);
		Body->IsCollisionEnabled = true;
		Added = true;
	}
	else if (!Added)
	{
		Body->IsCollisionEnabled = false;
		return;
	}

	if (Body->IsCollisionEnabled != NewEnabled)
	{
		if (NewEnabled)
		{
			Body->EnableCollision();
		}
		else
		{
			Body->DisableCollision();
		}
	}
}

bool engine::PhysicsComponent::GetCollisionEnabled() const
{
	if (!Body)
		return false;

	return Body->IsCollisionEnabled;
}

void engine::PhysicsComponent::SetActive(bool NewActive)
{
	if (!Body)
		return;

	if (!Added)
	{
		Body->IsActive = NewActive;
		return;
	}

	if (NewActive != Body->IsActive)
	{
		if (NewActive)
		{
			Body->Activate();
		}
		else
		{
			Body->Deactivate();
		}
	}
}

bool engine::PhysicsComponent::GetActive() const
{
	return Body->IsActive;
}

void engine::PhysicsComponent::Update()
{
}

void engine::PhysicsComponent::Clear()
{
	if (Body)
	{
		GetRootObject()->GetScene()->Physics.RemoveBody(Body);
		delete Body;
	}
	Body = nullptr;
}
