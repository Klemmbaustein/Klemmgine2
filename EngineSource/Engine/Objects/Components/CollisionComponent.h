#pragma once
#include "ObjectComponent.h"
#include <Engine/File/AssetRef.h>
#include <Engine/File/ModelData.h>
#include <Engine/Physics/Physics.h>

namespace engine
{
	class CollisionComponent : public ObjectComponent
	{
	public:
		void OnAttached() override;
		~CollisionComponent() override;
		void Update() override;

		void Load(AssetRef File, bool StartCollisionEnabled = true);
		void Load(GraphicsModel* Model, bool StartCollisionEnabled = true);
		physics::MeshBody* Body = nullptr;

		void SetCollisionEnabled(bool NewEnabled);
		bool GetCollisionEnabled() const;

		physics::PhysicsManager* GetManager();

		physics::PhysicsManager* Manager = nullptr;

	private:
		bool IsCollisionEnabled = true;
		Transform OldTransform;
		GraphicsModel* LoadedModel = nullptr;
	};
}