#include "Physics.h"
#include <Engine/Scene.h>
#include <Engine/Internal/JoltPhysics.h>
using namespace engine::physics;

engine::physics::PhysicsBody::PhysicsBody(BodyType NativeType, Transform BodyTransform, MotionType ColliderMovability, Layer CollisionLayers, ObjectComponent* Parent)
{
	this->Type = NativeType;
	this->BodyTransform = BodyTransform;
	this->ColliderMovability = ColliderMovability;
	this->CollisionLayers = CollisionLayers;
	this->Parent = Parent;
	if (Parent)
		this->Manager = &Parent->GetRootObject()->GetScene()->Physics;
}

void engine::physics::PhysicsBody::SetPosition(Vector3 NewPosition)
{
}

void engine::physics::PhysicsBody::SetPositionAndRotation(Vector3 NewPosition, Rotation3 NewRotation)
{
	Manager->PhysicsSystem->SetBodyPositionAndRotation(this, NewPosition, NewRotation);
}

void engine::physics::PhysicsBody::Activate()
{
	Manager->PhysicsSystem->SetBodyActive(this, true);
}

void engine::physics::PhysicsBody::Deactivate()
{
	Manager->PhysicsSystem->SetBodyActive(this, false);
}

void engine::physics::PhysicsBody::EnableCollision()
{
	Manager->PhysicsSystem->SetBodyCollisionEnabled(this, true);
}

void engine::physics::PhysicsBody::DisableCollision()
{
	Manager->PhysicsSystem->SetBodyCollisionEnabled(this, false);
}

void engine::physics::PhysicsBody::Scale(Vector3 ScaleMultiplier)
{
	Manager->PhysicsSystem->ScaleBody(this, ScaleMultiplier);
}

engine::physics::PhysicsManager::PhysicsManager(Scene* From)
{
	this->ParentScene = From;
}

void engine::physics::PhysicsManager::Init()
{
	internal::JoltInstance::InitJolt();
	this->PhysicsSystem = new internal::JoltInstance();
}

void engine::physics::PhysicsManager::Update()
{
	PhysicsSystem->Update();
}

HitResult engine::physics::PhysicsManager::RayCast(Vector3 Start, Vector3 End, Layer Layers, std::set<SceneObject*> ObjectsToIgnore)
{
	return PhysicsSystem->LineCast(Start, End, Layers, ObjectsToIgnore);
}

void engine::physics::PhysicsManager::AddBody(PhysicsBody* Body, bool StartActive, bool StartCollisionEnabled)
{
	PhysicsSystem->AddBody(Body, StartActive, StartCollisionEnabled);
}

void engine::physics::PhysicsManager::RemoveBody(PhysicsBody* Body)
{
	PhysicsSystem->RemoveBody(Body);
}

HitResult engine::physics::HitResult::GetAverageHit(std::vector<HitResult> Hits)
{
	if (!Hits.size())
	{
		return HitResult();
	}

	Vector3 HitNormal = Vector3(0, 0, 0);
	float MinDistance = INFINITY;

	float AvgDepth = 0, AvgDist = 0;

	Vector3 AvgPos = 0;

	for (auto& i : Hits)
	{
		HitNormal += i.Normal * i.Depth;
		MinDistance = std::min(i.Distance, MinDistance);
		AvgPos += i.ImpactPoint;
		AvgDepth += i.Depth;
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

	HitResult h;
	h.Normal = HitNormal;
	h.Distance = MinDistance;
	h.Depth = AvgDepth / (float)Hits.size();
	h.HitComponent = Hits[0].HitComponent;
	h.ImpactPoint = AvgPos;
	h.Hit = true;
	return h;
}

engine::physics::MeshBody::MeshBody(GraphicsModel* Mesh, Transform MeshTransform, MotionType ColliderMovability, Layer CollisionLayers, ObjectComponent* Parent)
	: PhysicsBody(BodyType::Mesh, MeshTransform, ColliderMovability, CollisionLayers, Parent)
{
	this->Model = Mesh;
}
