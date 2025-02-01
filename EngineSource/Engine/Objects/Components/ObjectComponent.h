#pragma once
#include <Engine/Graphics/Camera.h>
#include <Core/Transform.h>

namespace engine
{
	class SceneObject;

	class ObjectComponent
	{
	public:

		ObjectComponent();
		virtual ~ObjectComponent();
		virtual void OnAttached();
		virtual void Update();
		void UpdateAll();
		virtual void OnDetached();

		void DetachThis();

		void Detach(ObjectComponent* c);

		Vector3 Position;
		Rotation3 Rotation;
		Vector3 Scale = 1;
		std::vector<ObjectComponent*> Children;

		SceneObject* GetRootObject();
		ObjectComponent* GetRootComponent();

		SceneObject* ParentObject = nullptr;
		ObjectComponent* ParentComponent = nullptr;

		void Attach(ObjectComponent* Child);

		Transform GetWorldTransform();
		void UpdateTransform();

		SceneObject* RootObject = nullptr;

	protected:
		Transform GetParentTransform() const;

		Transform WorldTransform;
	private:
		void UpdateLogic();
	};
}