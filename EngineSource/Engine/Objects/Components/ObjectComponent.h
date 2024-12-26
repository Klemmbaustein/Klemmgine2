#pragma once
#include <Engine/Graphics/Camera.h>
#include <Engine/Transform.h>

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
		virtual void Draw(graphics::Camera* From);
		virtual void OnDetached();

		void DrawAll(graphics::Camera* From);

		Vector3 Position;
		Rotation3 Rotation;
		Vector3 Scale = 1;
		std::vector<ObjectComponent*> Children;

		SceneObject* ParentObject = nullptr;
		ObjectComponent* ParentComponent = nullptr;

		void Attach(ObjectComponent* Child);

		Transform GetWorldTransform();

	protected:
		Transform GetParentTransform();

		Transform WorldTransform;
	};
}