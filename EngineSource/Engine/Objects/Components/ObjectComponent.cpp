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
	UpdateTransform();
	UpdateLogic();
}

void engine::ObjectComponent::Draw(graphics::Camera* From)
{
}

void engine::ObjectComponent::OnDetached()
{
}

void engine::ObjectComponent::DetachThis()
{
	OnDetached();

	while (Children.size())
	{
		Detach(Children[0]);
	}
}

void engine::ObjectComponent::Detach(ObjectComponent* c)
{
	for (auto i = Children.begin(); i < Children.end(); i++)
	{
		if (*i == c)
		{
			Children.erase(i);
			c->OnDetached();
			delete c;
			return;
		}
	}
	ENGINE_UNREACHABLE();
}

void engine::ObjectComponent::DrawAll(graphics::Camera* From)
{
	Draw(From);
	for (ObjectComponent* i : Children)
	{
		i->DrawAll(From);
	}
}

SceneObject* engine::ObjectComponent::GetRootObject()
{
	if (ParentObject)
		return ParentObject;
	return ParentComponent->GetRootObject();
}

void engine::ObjectComponent::Attach(ObjectComponent* Child)
{
	Child->ParentComponent = this;
	Child->OnAttached();
	Children.push_back(Child);
}

Transform engine::ObjectComponent::GetWorldTransform()
{
	return GetParentTransform().Combine(Transform(Position, Rotation, Scale));
}

Transform engine::ObjectComponent::GetParentTransform() const
{
	if (ParentObject)
		return ParentObject->ObjectTransform;
	if (ParentComponent)
		return ParentComponent->WorldTransform;
	ENGINE_UNREACHABLE();
	return WorldTransform;
}

void engine::ObjectComponent::UpdateLogic()
{
	Update();
	for (ObjectComponent* i : Children)
	{
		i->UpdateLogic();
	}
}

void engine::ObjectComponent::UpdateTransform()
{
	WorldTransform = GetWorldTransform();
	for (ObjectComponent* i : Children)
	{
		i->UpdateTransform();
	}
}
