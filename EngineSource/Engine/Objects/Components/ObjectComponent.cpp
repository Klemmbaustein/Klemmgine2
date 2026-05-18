#include "ObjectComponent.h"
#include <Engine/Objects/SceneObject.h>
#include <Core/Error/EngineAssert.h>
using namespace engine;

engine::ObjectComponent::ObjectComponent()
{
}

engine::ObjectComponent::~ObjectComponent()
{
	for (auto& i : this->Children)
	{
		Detach(i);
	}
}

void engine::ObjectComponent::OnAttached()
{
}

void engine::ObjectComponent::Update()
{
}

void engine::ObjectComponent::UpdateAll()
{
	RootObject = GetRootObject();
	UpdateTransform();
	UpdateLogic();
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

SceneObject* engine::ObjectComponent::GetRootObject()
{
	if (ParentObject)
		return ParentObject;
	if (!ParentComponent)
		return nullptr;
	return ParentComponent->GetRootObject();
}

ObjectComponent* engine::ObjectComponent::GetRootComponent()
{
	if (ParentObject)
		return this;
	return ParentComponent ? ParentComponent->GetRootComponent() : nullptr;
}

void engine::ObjectComponent::Attach(ObjectComponent* Child)
{
	Child->ParentComponent = this;
	Child->OnAttached();
	Children.push_back(Child);

	auto Root = GetRootComponent();
	Child->RootObject = Child->GetRootObject();
	if (Root)
		Root->UpdateTransform();
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
	return Transform();
}

void engine::ObjectComponent::UpdateLogic()
{
	Update();
	for (ObjectComponent* i : Children)
	{
		i->UpdateLogic();
	}
}

bool engine::ObjectComponent::UpdateTransform(bool Dirty)
{
	bool DidUpdate = false;
	if (OldPosition != Position || OldScale != Scale || OldRotation != Rotation)
	{
		this->TransformDirty = true;
	}

	if (this->TransformDirty || Dirty)
	{
		WorldTransform = GetWorldTransform();
		DidUpdate = true;
	}
	for (ObjectComponent* i : Children)
	{
		i->UpdateTransform(this->TransformDirty || Dirty);
	}
	OldPosition = Position;
	OldScale = Scale;
	OldRotation = Rotation;
	this->TransformDirty = false;
	return DidUpdate;
}
