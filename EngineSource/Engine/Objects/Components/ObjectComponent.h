#pragma once
#include <Core/Transform.h>

namespace engine
{
	class SceneObject;

	class ObjectComponent
	{
	public:

		ObjectComponent();
		ObjectComponent(const ObjectComponent&) = delete;
		virtual ~ObjectComponent();
		virtual void OnAttached();
		virtual void Update();
		void UpdateAll();
		virtual void OnDetached();

		void DetachThis();

		void Detach(ObjectComponent* c);

		std::vector<ObjectComponent*> Children;

		SceneObject* GetRootObject();
		ObjectComponent* GetRootComponent();

		SceneObject* ParentObject = nullptr;
		ObjectComponent* ParentComponent = nullptr;

		void Attach(ObjectComponent* Child);

		Transform GetWorldTransform();
		virtual void UpdateTransform(bool Dirty = false);

		SceneObject* RootObject = nullptr;

		void SetPosition(const Vector3& NewPosition)
		{
			TransformDirty = true;
			Position = NewPosition;
		}

		void SetRotation(const Rotation3& NewRotation)
		{
			TransformDirty = true;
			Rotation = NewRotation;
		}

		void SetScale(const Vector3& NewScale)
		{
			TransformDirty = true;
			Scale = NewScale;
		}

		const Vector3& GetPosition()
		{
			return Position;
		}

		const Rotation3& GetRotation()
		{
			return Rotation;
		}

		const Vector3& GetScale()
		{
			return Scale;
		}

		Vector3 Position;
		Rotation3 Rotation;
		Vector3 Scale = 1;
		Transform WorldTransform;

	protected:
		Transform GetParentTransform() const;
		bool TransformDirty = true;

		Vector3 OldPosition;
		Rotation3 OldRotation;
		Vector3 OldScale = 1;

	private:
		void UpdateLogic();
	};
}