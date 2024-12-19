#pragma once
#include "SceneObject.h"
#include <Engine/File/ModelData.h>
#include <Engine/Graphics/Material.h>

namespace engine
{
	class MeshObject : public SceneObject
	{
	public:

		ENGINE_OBJECT(MeshObject, "Engine");

		ObjProperty<AssetRef> ModelName = ObjProperty<AssetRef>("Model", ".kmdl"_asset, this);

		std::vector<graphics::Material*> Materials;
		GraphicsModel* DrawnModel = nullptr;

		void LoadMesh(AssetRef File);

		virtual void Draw(graphics::Camera* From) override;
		void Begin() override;
		void Destroy() override;
	};
}