#include "CollisionComponent.h"
#include <Engine/Scene.h>
#include <Engine/Input.h>
#include <Core/Log.h>
using namespace engine::physics;
using namespace engine;

void engine::CollisionComponent::OnAttached()
{
}

engine::CollisionComponent::~CollisionComponent()
{
	if (Body && GetManager()->Active)
	{
		GetManager()->RemoveBody(Body);
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

	WorldTransform.Decompose(NewPosition, NewRotation, NewScale);
	OldTransform.Decompose(OldPosition, OldRotation, OldScale);

	if (OldScale == 0)
	{
		OldScale = 0.001f;
	}

	if (NewScale == 0)
	{
		NewScale = 0.001f;
	}

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

physics::PhysicsManager* engine::CollisionComponent::GetManager()
{
	return this->Manager ? this->Manager : &GetRootObject()->GetScene()->Physics;
}

bool engine::CollisionComponent::GetCollisionEnabled() const
{
	return IsCollisionEnabled;
}

void engine::CollisionComponent::Load(AssetRef File, bool StartCollisionEnabled)
{
	Load(GraphicsModel::GetModel(File), StartCollisionEnabled);
}

void engine::CollisionComponent::Load(GraphicsModel* Model, bool StartCollisionEnabled)
{
	this->LoadedModel = Model;

	if (!GetManager()->Active)
		return;

	auto Root = GetRootObject();
	if (Root)
	{
		Root->CheckTransform();
	}
	auto RootComp = GetRootComponent();
	if (RootComp)
	{
		RootComp->UpdateTransform();
	}

	if (!LoadedModel || LoadedModel->Data->Meshes.empty())
		return;

	Body = new MeshBody(LoadedModel, WorldTransform, physics::MotionType::Static, physics::Layer::Static, this);
	GetManager()->AddBody(Body, true, StartCollisionEnabled);
	OldTransform = WorldTransform;
	IsCollisionEnabled = StartCollisionEnabled;
}
