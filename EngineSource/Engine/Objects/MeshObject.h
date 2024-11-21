#pragma once
#include "SceneObject.h"
#include <Engine/Graphics/Model.h>

namespace engine
{
	class MeshObject : public SceneObject
	{
	public:
		graphics::ShaderObject* Shader = nullptr;
		string ModelName;
		graphics::Model* DrawnModel = nullptr;

		void LoadMesh(string Name);

		virtual void Draw(graphics::Camera* From) override;

		SerializedValue Serialize() override;
		void Begin() override;
	};
	REGISTER_OBJECT(MeshObject, engine::MeshObject);
}