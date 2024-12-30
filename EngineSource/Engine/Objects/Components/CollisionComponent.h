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

		void Load(AssetRef File, bool StartActive = true, bool StartCollisionEnabled = true);
		physics::MeshBody* Body = nullptr;

		void SetActive(bool NewActive);
		bool GetActive() const;

		void SetCollisionEnabled(bool NewEnabled);
		bool GetCollisionEnabled() const;

	private:
		bool IsActive = true;
		bool IsCollisionEnabled = true;
		Transform OldTransform;
		GraphicsModel* LoadedModel = nullptr;
	};
}