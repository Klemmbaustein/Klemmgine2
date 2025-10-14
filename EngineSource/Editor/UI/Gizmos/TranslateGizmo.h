#pragma once
#include <Engine/Objects/Components/MeshComponent.h>
#include <Engine/Objects/Components/CollisionComponent.h>

namespace engine::editor
{
	class Viewport;

	class TranslateGizmo
	{
	public:

		TranslateGizmo();
		~TranslateGizmo();

		TranslateGizmo(const TranslateGizmo&) = delete;

		void Update(Viewport* With);

		MeshComponent* GizmoMesh = nullptr;
		CollisionComponent* Collider = nullptr;

		bool HasGrabbedClick = false;

		physics::PhysicsManager Physics = physics::PhysicsManager(nullptr);

		Vector3 OldNormal;
		Vector3 CurrentPosition;
		float OldDistance = 0;
		int DraggedAxis = false;

		void SetVisible(bool NewVisible)
		{
			this->GizmoMesh->IsVisible = NewVisible;
		}
	};
}