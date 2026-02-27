#include "Physics.h"
#include <Engine/Objects/Components/CollisionComponent.h>
#include <Engine/Scene.h>
#include <Engine/Physics/Internal/JoltPhysics.h>
using namespace engine::physics;

PhysicsBody::PhysicsBody(BodyType NativeType, Transform BodyTransform,
	MotionType ColliderMovability, Layer CollisionLayers, ObjectComponent* Parent)
{
	this->Type = NativeType;
	this->BodyTransform = BodyTransform;
	this->ColliderMovability = ColliderMovability;
	this->CollisionLayers = CollisionLayers;
	this->Parent = Parent;

	if (Parent)
	{
		auto Collider = dynamic_cast<CollisionComponent*>(Parent);
		if (Collider)
		{
			this->Manager = Collider->GetManager();
		}
		else
		{
			auto obj = Parent->GetRootObject();
			if (obj)
			{
				this->Manager = &obj->GetScene()->Physics;
			}
			else
			{
				Log::Warn("Attempted to create a physics shape but it's component is not attached to any object!");
			}
		}
	}
}

engine::Vector3 engine::physics::PhysicsBody::GetPosition()
{
	return Manager->PhysicsSystem->GetBodyPosition(this);
}

std::pair<engine::Vector3, engine::Rotation3> engine::physics::PhysicsBody::GetPositionAndRotation()
{
	return Manager->PhysicsSystem->GetBodyPositionAndRotation(this);
}

void engine::physics::PhysicsBody::SetPosition(Vector3 NewPosition)
{
}

void engine::physics::PhysicsBody::SetRotation(Rotation3 NewRotation)
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

engine::physics::PhysicsManager::~PhysicsManager()
{
	delete this->PhysicsSystem;
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

HitResult engine::physics::PhysicsManager::RayCast(Vector3 Start, Vector3 End, Layer Layers,
	std::set<SceneObject*> ObjectsToIgnore)
{
	return PhysicsSystem->LineCast(Start, End, Layers, ObjectsToIgnore);
}

void engine::physics::PhysicsManager::AddBody(PhysicsBody* Body,
	bool StartActive, bool StartCollisionEnabled)
{
	PhysicsSystem->AddBody(Body, StartActive, StartCollisionEnabled);
}

void engine::physics::PhysicsManager::RemoveBody(PhysicsBody* Body)
{
	PhysicsSystem->RemoveBody(Body);
}

void engine::physics::PhysicsManager::PreLoadMesh(GraphicsModel* Mesh)
{
	PhysicsSystem->PreLoadMesh(Mesh);
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

	if (HitNormal == 0)
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

MeshBody::MeshBody(GraphicsModel* Mesh, Transform MeshTransform, MotionType ColliderMovability,
	Layer CollisionLayers, ObjectComponent* Parent)
	: PhysicsBody(BodyType::Mesh, MeshTransform, ColliderMovability, CollisionLayers, Parent)
{
	this->Model = Mesh;
}

HeightMapBody::HeightMapBody(const std::vector<float>& Samples, uint32 Size, Transform MeshTransform,
	MotionType ColliderMovability, Layer CollisionLayers, ObjectComponent* Parent)
	: PhysicsBody(BodyType::HeightMap, MeshTransform, ColliderMovability, CollisionLayers, Parent)
{
	this->Samples = Samples;
	this->Size = Size;
}
std::vector<HitResult> engine::physics::PhysicsBody::CollisionTest(Transform At,
	Layer Layers, std::set<SceneObject*> ObjectsToIgnore)
{
	return Manager->PhysicsSystem->CollisionTest(At, this, Layers, ObjectsToIgnore);
}

std::vector<HitResult> PhysicsBody::ShapeCast(Transform StartTransform, Vector3 EndPos,
	Layer Layers, std::set<SceneObject*> ObjectsToIgnore)
{
	return Manager->PhysicsSystem->ShapeCastBody(this, StartTransform, EndPos, Layers, ObjectsToIgnore);
}

SphereBody::SphereBody(Vector3 Position, Rotation3 Rotation, float Scale, MotionType ColliderMovability,
	Layer CollisionLayers, ObjectComponent* Parent)
	: PhysicsBody(BodyType::Sphere, Transform(Position, Rotation, Scale),
		ColliderMovability, CollisionLayers, Parent)
{
}

engine::physics::BoxBody::BoxBody(Vector3 Position, Rotation3 Rotation,
	Vector3 Extents, MotionType ColliderMovability, Layer CollisionLayers, ObjectComponent* Parent)
	: PhysicsBody(BodyType::Box, Transform(Position, Rotation, Extents),
		ColliderMovability, CollisionLayers, Parent)
{
}

engine::physics::CapsuleBody::CapsuleBody(Vector3 Position, Rotation3 Rotation, Vector2 Scale,
	MotionType ColliderMovability, Layer CollisionLayers, ObjectComponent* Parent)
	: PhysicsBody(BodyType::Capsule, Transform(Position, Rotation, Vector3(Scale.X, Scale.Y, 1)),
		ColliderMovability, CollisionLayers, Parent)
{
}
