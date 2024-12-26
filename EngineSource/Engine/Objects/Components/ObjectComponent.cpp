#include "ObjectComponent.h"
#include <Engine/Objects/SceneObject.h>
#include <Engine/Error/EngineAssert.h>
using namespace engine;

engine::ObjectComponent::ObjectComponent()
{
}

engine::ObjectComponent::~ObjectComponent()
{
}

void engine::ObjectComponent::OnAttached()
{
}

void engine::ObjectComponent::Update()
{
}

void engine::ObjectComponent::UpdateAll()
{
	for (ObjectComponent* i : Children)
	{
		i->UpdateAll();
	}
	WorldTransform = GetWorldTransform();
	Update();
}

void engine::ObjectComponent::Draw(graphics::Camera* From)
{
}

void engine::ObjectComponent::OnDetached()
{
}

void engine::ObjectComponent::DrawAll(graphics::Camera* From)
{
	Draw(From);
	for (ObjectComponent* i : Children)
	{
		i->DrawAll(From);
	}
}

void engine::ObjectComponent::Attach(ObjectComponent* Child)
{
	Children.push_back(Child);
}

Transform engine::ObjectComponent::GetWorldTransform()
{
	return GetParentTransform().Combine(Transform(Position, Rotation, Scale));
}

Transform engine::ObjectComponent::GetParentTransform()
{
	if (ParentObject)
		return ParentObject->ObjectTransform;
	if (ParentComponent)
		return ParentComponent->WorldTransform;
	ENGINE_UNREACHABLE();
	return WorldTransform;
}
