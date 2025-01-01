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
		physics::MeshBody* Body = nullptr;

		void SetCollisionEnabled(bool NewEnabled);
		bool GetCollisionEnabled() const;

	private:
		bool IsCollisionEnabled = true;
		Transform OldTransform;
		GraphicsModel* LoadedModel = nullptr;
	};
}