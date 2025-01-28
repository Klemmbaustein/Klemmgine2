#include "CollisionComponent.h"
#include <Engine/Scene.h>
#include <Engine/Input.h>
#include <Engine/Log.h>
using namespace engine::physics;

void engine::CollisionComponent::OnAttached()
{
}

engine::CollisionComponent::~CollisionComponent()
{
	if (Body)
	{
		GetRootObject()->GetScene()->Physics.RemoveBody(Body);
		delete Body;
	}
	if (LoadedModel)
	{
		GraphicsModel::UnloadModel(LoadedModel);
	}
}

void engine::CollisionComponent::Update()
{
	if (!Body || OldTransform.Matrix == WorldTransform.Matrix)
		return;

	Vector3 NewPosition, NewScale, OldPosition, OldScale;
	Rotation3 NewRotation, OldRotation;

	if (OldScale == 0)
	{
		OldScale = 0.001f;
	}

	if (NewScale == 0)
	{
		NewScale = 0.001f;
	}

	WorldTransform.Decompose(NewPosition, NewRotation, NewScale);
	OldTransform.Decompose(OldPosition, OldRotation, OldScale);

	Body->SetPositionAndRotation(NewPosition, NewRotation);

	Vector3 ScaleDifference = NewScale / OldScale;

	if (ScaleDifference != Vector3(1))
	{
		Body->Scale(ScaleDifference);
	}
	OldTransform = WorldTransform;
}

void engine::CollisionComponent::SetCollisionEnabled(bool NewEnabled)
{
	if (!Body)
		return;

	if (NewEnabled != IsCollisionEnabled)
	{
		if (NewEnabled)
			Body->EnableCollision();
		else
			Body->DisableCollision();
		IsCollisionEnabled = NewEnabled;
	}
}

bool engine::CollisionComponent::GetCollisionEnabled() const
{
	return IsCollisionEnabled;
}

void engine::CollisionComponent::Load(AssetRef File, bool StartCollisionEnabled)
{
	if (!GetRootObject()->GetScene()->Physics.Active)
		return;

	GetRootObject()->CheckTransform();
	GetRootComponent()->UpdateTransform();
	this->LoadedModel = GraphicsModel::GetModel(File);

	if (!LoadedModel)
		return;

	Body = new MeshBody(LoadedModel, WorldTransform, physics::MotionType::Static, physics::Layer::Static, this);
	GetRootObject()->GetScene()->Physics.AddBody(Body, true, StartCollisionEnabled);
	OldTransform = WorldTransform;
	IsCollisionEnabled = StartCollisionEnabled;
}
