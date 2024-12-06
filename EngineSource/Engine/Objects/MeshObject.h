#pragma once
#include "SceneObject.h"
#include <Engine/File/ModelData.h>
#include <Engine/Graphics/ShaderObject.h>

namespace engine
{
	class MeshObject : public SceneObject
	{
	public:

		ENGINE_OBJECT(MeshObject, "Engine");

		ObjProperty<AssetRef> ModelName = ObjProperty<AssetRef>("Model", ".kmdl"_asset, this);

		graphics::ShaderObject* Shader = nullptr;
		GraphicsModel* DrawnModel = nullptr;

		void LoadMesh(string Name);

		virtual void Draw(graphics::Camera* From) override;
		void Begin() override;
	};
}