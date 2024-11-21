#pragma once
#include "SceneObject.h"
#include <Engine/Graphics/Model.h>

namespace engine
{
	class MeshObject : public SceneObject
	{
	public:

		ENGINE_OBJECT(MeshObject, "Path/Test");

		ObjProperty<AssetRef> ModelName = ObjProperty<AssetRef>("model", AssetRef(""), this);

		graphics::ShaderObject* Shader = nullptr;
		graphics::Model* DrawnModel = nullptr;

		void LoadMesh(string Name);

		virtual void Draw(graphics::Camera* From) override;
		void Begin() override;
	};
}